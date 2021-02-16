IRC Client Daemon CONTRIBUTING GUIDE
====================================

Read this guide if you want to contribute to irccd. The purpose of this
document is to describe the steps to submit a patch.

You may submit a patch when:

- You want to fix a bug / typo,
- You want to add a new feature,
- You want to change something.

There a lot of steps before submitting a patch. First, be sure to respect the
style defined in the STYLE.md file. We never accept patches that do not match
the rules.

Subscribe to the mailing list
-----------------------------

Discussion and patches are sent to the *irccd@malikania.fr* mailing list.
You need to subscribe by dropping a mail to
*irccd+subscribe@malikania.fr* first.

Enable patchbomb extension
--------------------------

While this step is optional, it brings the `hg email` command which makes most
of your submission for you.

To enable it, add the following into your .hgrc (you may also use the hgrc file
from the repository in .hg/hgrc).

    [extensions]
    patchbomb =

Then, you need to specify a mail server, if you want to use smtp, you can use
something like this:

    [email]
    from = Your Name <youraddress@yourdomain.tld>
    to = irccd@malikania.fr

    [smtp]
    host = yourdomain.tld
    port = 587
    tls = starttls
    username = your_account
    password = your_password

Note: the password is optional, if not set it will be asked each time you run
the `hg email command`.

More options are available, see:

- `hg help hgrc.email`,
- `hg help hgrc.smtp`,
- `hg help patchbomb`
- `hg help email`

### Note to GMail users

By default, your GMail account may use 2-steps authentication which causes
troubles with the `hg email` command, you must create a specific application
password.

1. Go to https://security.google.com/settings/security/apppasswords
2. Create an application password, it will be auto generated,
3. Use this password or store it directly in the `smtp.password` option.

Use the following settings:

    [smtp]
    host = gmail.com
    port = 587
    tls = starttls
    username = your_account@gmail.com
    password = the_generated_application_password

Create your patch
-----------------

Usually, when you create a patch, you should have your own copy of irccd
in your directory.

The following steps assumes that you have already cloned the irccd
repository somewhere.

Note: the recommended way is to create one unique revision.

### Commit messages

Commit messages are written using the following syntax:

    topic: short message less than 80 characters

    Optional additional description if needed.

Replace `topic` with one of the following:

- **cmake**: for the build system,
- **doc**: for the documentation,
- **irccd**: irccd libraries and frontend,
- **irccdctl**: irccd libraries and frontend.
- **misc**: for miscellaneous files,
- **plugin xyz**: plugin named xyz.
- **release**: release management,
- **tests**: for the unit tests,

### Quick way

If you plan to create a very small patch that consists of several lines, you can
use the following way by disabling the @ bookmark to avoid moving it.

    $ hg pull           # fetch last changesets
    $ hg up @           # update to the last revision
    $ hg book -i @      # disable the @ bookmark (optional but recommended)
    (edit some files)
    $ hg commit         # create a unique revision
    $ hg email -r .     # send a mail about the current revision (interactive)

### Bookmark way

We use Mercurial bookmarks as our workflow but we do share only @ bookmark
except when a long feature is being developed in parallel. Otherwise bookmarks
stay locally most of the time.

When you start working on a new feature, you **must** always start from the @
bookmark.

You can use this workflow if you plan to create a patch that consists of
multiple revisions.

Example:

    $ hg pull
    $ hg up @
    $ hg book feature-xyz
    (work)
    $ hg commit
    (work)
    $ hg commit
    $ hg email -r first:last

Here, you must specify **first** and **last** as the initial and last revisions
respectively. You can check these revisions using `hg log` (also try `hg log -G`
or the nice TortoiseHg interface).

Example, I've started to work on an a feature named **feature-xyz**, the log
looks like this:

    changeset:   22:3fb15d8fc454
    bookmark:    feature-xyz
    tag:         tip
    user:        François Jean <fj@gmail.com>
    date:        Thu Dec 08 16:08:40 2016 +0100
    summary:     topic: some other changes

    changeset:   21:f27e577c5504
    user:        François Jean <fj@gmail.com>
    date:        Thu Dec 08 16:03:06 2016 +0100
    summary:     topic: some changes

    changeset:   20:777023816ff9
    bookmark:    @
    user:        David Demelier <markand@malikania.fr>
    date:        Thu Dec 08 16:02:26 2016 +0100
    summary:     misc: fix a bug

The two revisions I want to export are 21 and 22, so I use `hg email -r 21:22`,
once done, see the section below.

Additional topics
-----------------

### Your patch is accepted

The safest choice is to just pull from the central repository and update to the
@ bookmark.

    $ hg pull
    $ hg up @

You can also call `hg rebase` (from rebase extension) to move your revisions on
top of upstream. If the patches were incorporated verbatim, they will be safely
discarded automatically.

    $ hg pull
    $ hg up @
    $ hg rebase -b feature-xyz -d @
    $ hg book -d feature-xyz

If you didn't created a bookmark replace **feature-xyz** with your revision
number.

Finally, if you prefer to remove the revisions you have created, use `hg strip`
like explained in the see section below.

### Your patch is discarded

For some reasons, your patch can not be integrated within the official
repository, you can remove the revisions you have commited or keep them.

If you want to remove the revisions, you can use the `hg strip` command (from
the strip extension).

Warning: it will **remove** the revisions from history so use with care.

    $ hg strip -r 21:22         # using the example above
    $ hg book -d feature-xyz    # delete the bookmark

Newer versions of Mercurial support `-B` argument:

    $ hg strip -B feature-xyz   # shortcut

You can just go back on the @ bookmark as it's the safest choice.

    $ hg pull                   # fetch changesets
    $ hg up @                   # update to @

### How to merge upstream code to continue my patch

Sometimes when you started working on a topic, you may need to pull changes from
the repository. The idea is to pull the changes and rebase your work on top of
it.

You must run these commands while your bookmark is active

    $ hg up feature-xyz
    $ hg pull -B @
    $ hg rebase -b feature-xyz -d @

### I forgot to create a bookmark and accidentally moved the @ bookmark

If you forgot to create a custom bookmark or disable @ before committing, you
may have moved the @ bookmark in your repository. The `hg pull` command can
recover it.

First, we create it now to point at your local revisions (optional).

    $ hg book feature-xyz

Then, put it where it should be.

    $ hg pull -B @

Now @ will be placed to the same revision as the central repository. If some
changesets have been pulled, you may look at the previous topic to rebase your
work on top of it.
