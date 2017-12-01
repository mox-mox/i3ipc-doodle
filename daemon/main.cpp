#include <iostream>
#include <stdio.h>


#include <libnotify/notify.h>

#include "confuse.h"



int main(void)
{
	std::cout<<"Hello world!"<<std::endl;

	cfg_opt_t opts[] =
	{
		CFG_STR("target", "No One", CFGF_NONE),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	if(cfg_parse(cfg, "hello.conf") == CFG_PARSE_ERROR)
		return 1;

	printf("Hello, %s!\n", cfg_getstr(cfg, "target"));

	cfg_free(cfg);






	notify_init("doodle");
	NotifyNotification* vi3failed = notify_notification_new("Doodle", "Hello World!", "dialog-information");
	//notify_notification_set_urgency(vi3failed, NOTIFY_URGENCY_CRITICAL);
	notify_notification_set_timeout(vi3failed, 10000);
	notify_notification_show(vi3failed, NULL);
	g_object_unref(G_OBJECT(vi3failed));
	notify_uninit();

	return 0;
}
