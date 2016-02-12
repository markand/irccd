# Plugins

Irccd can be extended with JavaScript plugins. This chapter will tell you how plugins work within irccd and how to
create your first plugin.

This chapter covers also some things to do and to avoid in plugins.

## Why JavaScript?

You may wonder why JavaScript was chosen in irccd. Originally, irccd used Lua as the scripting language but for many
reasons, it has been replaced with JavaScript.

However, many aspects between Lua and JavaScript are similar:

  - Both languages are extremly small with very light API,
  - It is easy to sandbox the interpreter for security reasons,
  - It is very easy to implement your own API from C++ code.

The current JavaScript interpreter is powered by [Duktape][duktape].

[duktape]: http://duktape.org
