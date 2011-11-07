#ifndef __epics_like_h
#define __epics_like_h 1

#ifdef __cplusplus
extern "C" {
#endif

/* defines for the choice fields */
/* ALARM SEVERITIES - NOTE: must match defs in choiceGbl.ascii GBL_ALARM_SEV */
#define NO_ALARM                0x0
#define MINOR_ALARM             0x1
#define MAJOR_ALARM             0x2
#define INVALID_ALARM           0x3
#define ALARM_NSEV              INVALID_ALARM+1

/* ALARM STATUS  -NOTE: must match defs in choiceGbl.ascii GBL_ALARM_STAT */
/* NO_ALARM = 0 as above */
#define	READ_ALARM		1
#define	WRITE_ALARM		2
/* ANALOG ALARMS */
#define	HIHI_ALARM		3
#define	HIGH_ALARM		4
#define	LOLO_ALARM		5
#define	LOW_ALARM		6
/* BINARY ALARMS */
#define	STATE_ALARM		7
#define	COS_ALARM		8
/* other alarms */
#define COMM_ALARM		9
#define	TIMEOUT_ALARM		10
#define	HW_LIMIT_ALARM		11
#define	CALC_ALARM		12
#define	SCAN_ALARM		13
#define	LINK_ALARM		14
#define	SOFT_ALARM		15
#define	BAD_SUB_ALARM		16
#define	UDF_ALARM		17
#define	DISABLE_ALARM		18
#define	SIMM_ALARM		19
#define	READ_ACCESS_ALARM	20
#define	WRITE_ACCESS_ALARM	21
#define ALARM_NSTATUS		WRITE_ACCESS_ALARM + 1

#ifdef __cplusplus
}
#endif

#endif
