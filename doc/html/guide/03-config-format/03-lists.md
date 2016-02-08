## The list construct

When requested, an option can have multiples values in a list. The syntax uses parentheses and values are separated
by commas.

If the list have only one value, you can just use a simple string.

### Examples

<div class="alert alert-success" role="alert">
**Example**: two servers defined in a rule

````ini
[rule]
servers = ( "server1", "server2" )
````
</div>

<div class="alert alert-success" role="alert">
**Example**: only one server

````ini
[rule]
servers = "only-one-server"
````
</div>

<div class="alert alert-info" role="alert">
**Note:** spaces are completely optional.
</div>
