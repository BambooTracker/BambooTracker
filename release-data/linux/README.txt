Notes specific to the Linux release
###################################

If you're on Ubuntu, or another Debian-based distribution, and you're having
issues with the binary from the release page, then here are some things you
may try to fix them:

Sysctl for unprivileged user namespace clone
############################################

Debian, and distros derived from it, have a sysctl setting called
`kernel.unprivileged_userns_clone` which allows the system administrator to
control the availability of something called "User Namespaces".

For information on User Namespaces, see:

man 7 user_namespaces

With this feature disabled (which is the default), the binary will usually
not work and complain on startup with something like the following:

Run the following to enable unprivileged namespace use:
sudo bash -c "sysctl -w kernel.unprivileged_userns_clone=1 ; echo kernel.unprivileged_userns_clone=1 > /etc/sysctl.d/nix-user-chroot.conf"

To resolve this, just run the suggested command.

AppArmor user namespace permissions
###################################

On Ubuntu specifically (since 23.10), the sysctl setting
`kernel.apparmor_restrict_unprivileged_userns` was introduced to limit
the accessibility of User Namespaces. When trying to run BambooTracker, this
setting will cause the mounting of a lot of directories to fail.

To read up on what exactly this extra setting is for, and why it exists:
https://ubuntu.com/blog/ubuntu-23-10-restricted-unprivileged-user-namespaces

Instead of globally disabling this security measure, it is recommended to give
the binary in question the necessary AppArmor permissions to use
User Namespaces. To do this:

1. Take note of the full path of your extracted BambooTracker binary.

   For example:
   /home/alice/Downloads/BambooTracker-v0.6.5-linux-64bit/bin/BambooTracker

2. Create a new file at this location:

   /etc/apparmor.d/BambooTracker

   ... and put into it the following text (the start and end of what needs
   to be written is marked with ```, do not include this marker):

   ```
   abi <abi/4.0>,

   include <tunables/global>

   <BAMBOOTRACKER_PATH> flags=(default_allow) {
     userns,
   }
   ```

   ... and replace <BAMBOOTRACKER_PATH> with the path to the binary from step 1.

   Note: If you move the binary to somewhere else after completing this, you'll
         have adjust this path in the AppArmor file.

3. Make AppArmor reload its definitions, so it picks up the new file:

   sudo systemctl reload apparmor.service

The binary should now be working.
