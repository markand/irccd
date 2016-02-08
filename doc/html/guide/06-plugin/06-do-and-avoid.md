## Do and do not

There are things which should be avoided if possible.

### Error and load

Since irccd 1.1, one should not write code outside JavaScript supported events functions. Internally, irccd store the
plugin information the complete read. If a plugin has an syntax error or a bad API call, irccd looks for the plugin
metadata information and since it is not currently stored, it generate the "unitialized state" error.

### Handling error

There are two ways of handling error.

#### Your plugin can't continue running

If you need a file, specific resource so your plugin.

If you call the error function while you are in the `onLoad` callback, the plugin is not added to the registry.
However, if the function is called in any other event, the plugin remains in the list.

#### Your plugin has errors but can run

If for instance, the plugin has errors but can still run for any reason. One should use the `Irccd.Logger` API. The
function in this API will write a message in the irccd output.

**Example**

````javascript
function onCommand()
{
    if (something_is_wrong)
        Irccd.Logger.warning("error condition")
}
````

This will output to the irccd log something like:

````
plugin foo: error condition
````
