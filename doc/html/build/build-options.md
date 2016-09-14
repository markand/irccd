---
title: Build options
guide: yes
---

# Customizing the build

You can configure some features when compiling irccd.

## Disabling JavaScript

You can disable JavaScript support.

````nohighlight
$ cmake .. -DWITH_JS=Off
````

## Disabling SSL

You can disable OpenSSL support, it is automatically unset if OpenSSL is not found.

<div class="alert alert-warning" role="alert">
**Warning**: this is not recommended.
</div>

````nohighlight
$ cmake .. -DWITH_SSL=Off
````

## Disabling all documentation

You can disable all the documentation.

````nohighlight
$ cmake .. -DWITH_DOCS=Off
````

See below to disable only specific parts of the documentation.

## Disabling HTML documentation

By default if Pandoc is available, the HTML documentation is built, you can disable it.

````nohighlight
$ cmake .. -DWITH_HTML=Off
````

## Disabling man pages

You can disable installation of manuals.

````nohighlight
$ cmake .. -DWITH_MAN=Off
````

## Installation path

Sometimes, you may need to install irccd over other place, for that, you can
specify the prefix where to install files.

On Unix systems, it's usually **/usr/local** and **C:/Program Files/Irccd** on Windows.

To change this, use the following:

````nohighlight
$ cmake .. -DCMAKE_INSTALL_PREFIX=/opt/some/directory
````

## Manual pages path

By default, irccd use **${CMAKE_INSTALL_PREFIX}/share/man** for manual pages. Some systems use different one.

For example, on FreeBSD the typical use would be:

````nohighlight
$ cmake .. -DWITH_MANDIR=/usr/local/man
````
