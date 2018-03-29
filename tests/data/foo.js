info = {
    author: "David Demelier <markand@malikania.fr>",
    license: "ISC",
    summary: "foo",
    version: "0.0"
};

function onLoad()
{
}

function onReload()
{
    Irccd.Plugin.config["reloaded"] = "true";
}
