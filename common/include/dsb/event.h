#ifndef _EVENT_H_
#define _EVENT_H_

#define MAX_EVENT_PARAMS	2

enum EventType {
	EVENT_GET=0,
	EVENT_SET,
	EVENT_DEFINE,
	EVENT_DELETE,
	EVENT_DEP,
	EVENT_INVALID
};

struct Event {
	EventType type;
	HARC dest;
};

struct EventExt {
	EventType type;
	HARC dest;
	NID p[MAX_EVENT_PARAMS];
};

int dsb_event_init();
int dsb_event_final();
int dsb_event_check(const Event *evt);
Event *dsb_event_poolAllocate();
void dsb_event_poolFree(Event *evt);
int dsb_event_length(const Event *evt);

#endif

