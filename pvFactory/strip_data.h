// -*- C++ -*-
// EDM strip data
//
// kasemir@lanl.gov

#ifndef __STRIP_DATA__
#define __STRIP_DATA__

#include<stddef.h>
#include<time.h>
#include<pthread.h>
#include"lin_interpol.h"

// For given number of channels,
// hold stripchart data/history in buckets
// for a total of given seconds.
class StripData
{
public:
    StripData(size_t channel_count, size_t bucket_count,
              double seconds, time_t now, unsigned long nano);
    ~StripData();

    class Bucket
    {
    public:
        typedef enum
        {
            empty,
            sampled,
            estimated
        } BucketState;

        Bucket()
        {   state=empty; }
        void clear()
        {   state=empty; }
        void guess(double value)
        {
            if (state != sampled)
            {
                mini = last = maxi = value;
                state = estimated;
            }
        }
        void sample(double value)
        {
            if (state!=sampled)
            {
                mini = last = maxi = value;
                state = sampled;
            }
            else
            {
                if (mini > value)
                    mini = value;
                if (maxi < value)
                    maxi = value;
                last = value;
            }
        }
        BucketState state;
        double mini, last, maxi;
    };

    void updateEnd(time_t time, unsigned long nano);
    void addSample(size_t channel,
                   time_t time, unsigned long nano, double value);
    // Clear entries last_added+1 ... end
    void discontinue(size_t channel);

    // When reading buckets, lock/unlock has to be called
    void lock();
    void unlock();
    size_t getBucketCount() const;
    const Bucket *getBucket(size_t channel, size_t b);
    double getBucketSecs(size_t b) const;
private:
    pthread_mutex_t mutex;
    size_t channel_count;
    size_t bucket_count;
    double seconds;
    double end_secs;     // secs & nanosecs, start = end - seconds
    
    LinearTransformation sec2bucket;
    Bucket **buckets;   // for [-seconds .. 0], relative to end
    size_t bucket0;     // Ring, this is the first entry (-seconds)
    size_t *last_added; // index: last real sample bucket from addSample
    size_t *last_entry; // index: last used bucket, initially==bucket_count

    size_t ringindex(size_t b) const;
};

// inlines ----------------------------
inline void StripData::lock()
{
    if (pthread_mutex_lock(&mutex) != 0)
        fprintf (stderr,"StripData mutex error: lock\n");
    
}

inline void StripData::unlock()
{
    if (pthread_mutex_unlock(&mutex) != 0)
        fprintf (stderr,"StripData mutex error: unlock\n");
}

inline size_t StripData::getBucketCount() const
{   return bucket_count; }

inline size_t StripData::ringindex(size_t b) const
{   return (b + bucket0) % bucket_count; }

inline const StripData::Bucket *StripData::getBucket(size_t channel, size_t b)
{   return &buckets[channel][ringindex(b)]; }

inline double StripData::getBucketSecs(size_t b) const
{   return sec2bucket.inverse(b); }

#endif

