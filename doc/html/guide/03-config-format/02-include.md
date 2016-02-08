## The @include statement

Irccd adds an extension to this format by adding an `@include` keyword which let you splitting your configuration file.

<div class="alert alert-info" role="alert">
**Note:** this `@include` statement must be at the beginning of the file and must be surrounded by quotes if the file
name has spaces.
</div>

You can use both relative or absolute paths. If relative paths are used, they are relative to the current file being
parsed.

### Example

````ini
@include "rules.conf"
@include "servers.conf"

[mysection]
myoption = "1"
````
