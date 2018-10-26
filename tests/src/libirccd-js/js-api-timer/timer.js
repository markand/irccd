count = 0;

function onLoad()
{
	if (typeof (type) === "undefined")
		throw Error("global timer type not defined");

	t = new Irccd.Timer(type, 50, function () {
		count += 1;
	});

	t.start();
}
