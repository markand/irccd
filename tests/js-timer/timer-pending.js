count = 0;

function onLoad()
{
    t = new Irccd.Timer(Irccd.Timer.Repeat, 500, function () {
        count += 1;
    });

    t.start();

    /* Force the plugin to wait so that timer push some events in the irccd's queue */
    Irccd.System.sleep(3);
    t.stop();
}
