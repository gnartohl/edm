// -*- C++ -*-
// EDM strip data
//
// kasemir@lanl.gov

#include<stdio.h>
#include"strip_data.h"


StripData::StripData(size_t _channel_count,
                     size_t _bucket_count,
                     double _seconds,
                     time_t now, unsigned long nano)
{
    pthread_mutex_init(&mutex, 0);
    channel_count = _channel_count;
    bucket_count  = _bucket_count;
    seconds       = _seconds;
    end_secs      = (double)now + nano/1e9;
    
    sec2bucket.setSource(-seconds, 0);
    sec2bucket.setDestination(0, bucket_count);

    buckets = new Bucket *[channel_count];
    last_added = new size_t[channel_count];
    last_entry = new size_t[channel_count];
    for (size_t i=0; i<channel_count; ++i)
    {
        buckets[i] = new Bucket[bucket_count];
        last_added[i] = bucket_count;
        last_entry[i] = bucket_count;
    }
    bucket0 = 0;
}

StripData::~StripData()
{
    for (size_t i=0; i<channel_count; ++i)
        delete [] buckets[i];
    delete [] last_entry;
    delete [] last_added;
    delete [] buckets;
    pthread_mutex_destroy(&mutex);
}

void StripData::updateEnd(time_t time, unsigned long nano)
{
    lock();
    double t = (double)time + nano/1e9;
    double dt = t - end_secs;
    if (dt <= 0)
    {
        unlock();
        return;
    }
    int scroll = (int)(sec2bucket.transform(dt) + 0.5) - bucket_count;
    if (scroll < 1)
    {
        unlock();
        return;
    }

    size_t channel, i;
    double v;
    for (channel=0; channel<channel_count; ++channel)
    {
        // clear first 'scroll' items=new 'end' buckets
        for (i=0; i<(size_t)scroll; ++i)
            buckets[channel][ringindex(i)].clear();

        // guess that last_entry stays valid up to new end_secs
        if (last_entry[channel] < bucket_count)
        {
            if (buckets[channel][ringindex(last_entry[channel])].state
                != Bucket::empty)
            {
                v = buckets[channel][ringindex(last_entry[channel])].last;
                for (i=1; i<=(size_t)scroll; ++i)
                    buckets[channel][ringindex(last_entry[channel]+i)].guess(v);
            }
        }
    }

    bucket0 = (bucket0+scroll) % bucket_count;
    end_secs = t;

    // last_entry stays since it was filled to,
    // last_added (real value from server) scolls back
    for (channel=0; channel<channel_count; ++channel)
    {
        if (last_added[channel] < bucket_count)
        {
            if (last_added[channel] >= (size_t)scroll)
                last_added[channel] -= scroll;
            else
                last_added[channel] = bucket_count;
        }
    }
    
    unlock();
}

void StripData::addSample(size_t channel,
                          time_t t, unsigned long nano, double value)
{
    size_t i;
    lock();
    // t -> -seconds .. 0
    double seconds_back = (double)t + nano/1e9 - end_secs;
    int b = (size_t)(sec2bucket.transform(seconds_back)+0.5);
    if (b < 0) // too old
    {
        unlock();
        return;
    }
    if ((size_t)b >= bucket_count) // future -> now
        b = bucket_count-1;
    // b = 0... bucket_count-1, bucket index for new value

    // Repeat last known entry up to time t
    if (last_added[channel] < (size_t)b)
    {
        double v = buckets[channel][ringindex(last_added[channel])].last;
        for (i=last_added[channel]+1; i<(size_t)b; ++i)
            buckets[channel][ringindex(i)].sample(v);
    }

    // add new entry
    buckets[channel][ringindex(b)].sample(value);
    last_entry[channel] = last_added[channel] = b;
    
    // remove entries from here until "last_added"
    // which might be a result of scrolling
    for (i=b+1; i<bucket_count; ++i)
        buckets[channel][ringindex(i)].clear();
    
    unlock();
}

void StripData::discontinue(size_t channel)
{
    lock();
    size_t i;
    for (i=last_entry[channel]+1; i<bucket_count; ++i)
        buckets[channel][ringindex(i)].clear();
    last_entry[channel] = last_added[channel] = bucket_count;
    unlock();
}
