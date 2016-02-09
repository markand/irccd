## The list construct

When requested, an option can have multiples values in a list. The syntax uses parentheses and values are separated
by commas.

If the list have only one value, you can just use a simple string.

### Examples

<div class="panel panel-success">
 <div class="panel-heading">**Example:** two servers defined in a rule</div>
 <div class="panel-body">
````ini
[rule]
servers = ( "server1", "server2" )
````
 </div>
</div>

<div class="panel panel-success">
 <div class="panel-heading">**Example:** only one server</div>
 <div class="panel-body">
````ini
[rule]
servers = "only-one-server"
````
 </div>
</div>

<div class="alert alert-info" role="alert">
**Note:** spaces are completely optional.
</div>
