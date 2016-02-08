count = 0;

function onLoad()
{
	t = new Irccd.Timer(Irccd.Timer.Single, 500, function () {
		count += 1;
	});

	t.start();
}
