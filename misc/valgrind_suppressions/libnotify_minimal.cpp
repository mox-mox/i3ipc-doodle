#include <libnotify/notify.h>
#include <iostream>

int main(int argc, char * argv[] ) 
{
    notify_init("Sample");
    NotifyNotification* n = notify_notification_new ("Hello world", "Some text for the second line...", 0);
    notify_notification_set_timeout(n, 1000);
    notify_notification_show(n, 0);
	notify_uninit();
    return 0;
}
