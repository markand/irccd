irccd CONTRIBUTING GUIDE
========================

Read this guide if you want to contribute to irccd. The purpose of this
document is to describe the steps to submit a patch.

First, make sure to follow the project coding style in the [STYLE.md][] file if
present.

Note to AI powerusers
---------------------

This project does not accept anything that is produced by an AI agent which
includes code content, commit message or mail body or any kind of assets.

If you still do so keep in mind that your submitted content will be ignored
without any response.

Use your brain.

Optional: enable patchbomb extension
------------------------------------

While this step is optional, it brings the `hg email` command which makes most
of your submission for you.

To enable it, add the following into your .hgrc (you may also use the hgrc file
from the repository in .hg/hgrc).

    [extensions]
    patchbomb =

Then, you need to specify a mail server. If you want to use smtp, you can use
something like this:

    [email]
    from = Your Name <youraddress@yourdomain.tld>
    to = markand@malikania.fr

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

When you create a patch, you should have your own copy of the repository in your
directory.

Note: the recommended way is to create one unique revision.

### Commit messages

Commit messages are written using the following syntax:

    topic: short message less than 80 characters

    Optional additional description if needed.

Replace `topic` with one of the following:

- **make**: for the build system
- **cmake**: for the build system
- **doc**: for the documentation
- **misc**: for miscellaneous files
- **project**: for general project messages
- **tests**: for the unit tests

More topics may be used within the project. Check existing commit logs for an
overview.

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

You can use Mercurial bookmarks to create a divergent marker from the upstream
`@` bookmark pointing at the current default branch revision.

You **must** always start from the @ bookmark.

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
    $ hg email -B feature-xyz

Additional topics
-----------------

### Your patch is accepted

The safest choice is to just pull from the central repository and update to the
@ bookmark.

    $ hg pull           # pull everything
    $ hg pull -B @      # bring back @ bookmark (optional but recommended)
    $ hg up @           # update to the last revision

Finally, if you prefer to remove the revisions you have created, use `hg strip`
like explained in the see section below.

### Your patch is discarded

For some reasons, your patch can not be integrated within the official
repository, you can remove the revisions you have commited or keep them.

If you want to remove the revisions, you can use the `hg strip` command (from
the strip extension).

Warning: it will **remove** the revisions from history so use with care.

    $ hg strip -B feature-xyz   # using the example above

You can just go back on the @ bookmark as explained in previous section.

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
