#ifdef __LINUX_ALSA__
#  include <alsa/asoundlib.h>
#endif

#ifdef __LINUX_PULSE__
#  include <pulse/error.h>
#  include <pulse/simple.h>
#  include <pulse/pulseaudio.h>
#endif

#ifdef __UNIX_JACK__
#  include <jack/jack.h>
#endif

#ifdef __LINUX_OSS__
#  ifdef OSS_UNDER_SYS
#    include <sys/soundcard.h>
#  else
#    include <soundcard.h>
#  endif
/* Make sure this is OSS4 */
oss_sysinfo sysinfo;
oss_audioinfo ainfo;
#endif

int main() {
#ifdef __UNIX_JACK__
#  ifdef JACK_HAS_PORT_RENAME
  jack_port_rename(NULL, NULL, NULL);
#  else
  jack_port_set_name(NULL, NULL);
#  endif
#endif
	return 0;
}
