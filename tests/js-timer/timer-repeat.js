count = 0;

function onLoad()
{
	t = new Irccd.Timer(Irccd.Timer.Repeat, 500, function () {
		count += 1;
	});

	t.start();
}
