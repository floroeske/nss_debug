# Modified nss_debug to do localsinglename lookups

## Description

A small module for the NSS (Name Service Switch) to lookup hostnames as singlenames with mdns. Probably not advised to do so. But I think it's convenient. More of an experiment.

This NSS module just takes 'myhostname' and does a new look-up for 'myhostname.local'.

## Installation

```
make
sudo make install
```

## Configuration

Add debug to your hosts entry in your /etc/nsswitch.conf file.

e.g. /etc/nsswitch.conf
```
hosts:          files debug mdns4_minimal [NOTFOUND=return] dns
```

The debug modules for group, passwd and shadow are still there. See the original repo on how to configure them.
