Gamelists
=========

The gamelist.xml file for a system defines metadata for a system's games, such as a name, image (like a screenshot or box art), description, release date, and rating.

ES will check three places for a gamelist.xml in the following order, using the first one it finds:
* `[SYSTEM_PATH]/gamelist.xml`
* `~/.emulationstation/gamelists/[SYSTEM_NAME]/gamelist.xml`
* `/etc/emulationstation/gamelists/[SYSTEM_NAME]/gamelist.xml`

An example gamelist.xml:
```xml
<gameList>
	<game>
		<path>/home/pi/ROMs/nes/mm2.nes</path>
		<name>Mega Man 2</name>
		<desc>Mega Man 2 is a classic NES game which follows Mega Man as he murders eight robot masters in cold blood.</desc>
		<image>~/.emulationstation/downloaded_images/nes/Mega Man 2-image.png</image>
	</game>
</gameList>
```

Everything is enclosed in a `<gameList>` tag.  The information for each game or folder is enclosed in a corresponding tag (`<game>` or `<folder>`).  Each piece of metadata is encoded as a string.


Reference
=========

(if you suspect this section is out of date, check out `src/MetaData.cpp`)

There are a few types of metadata:

* `string` - just text.
* `image_path` - a path to an image. This path should be either the absolute to the image, a path relative to the system games folder that starts with "./" (e.g. `./mm2_image.png`), or a path relative to the home directory that starts with "~/" (e.g. `~/.emulationstation/downloaded_images/nes/mm2-image.png`).  Images will be automatically resized by OpenGL to fit the corresponding `<image>` tag in the current theme.  Smaller images will load faster, so try to keep resolution low!
* `video_path` - a path to a video. Similar to `image_path`.
* `float` - a floating-point decimal value (written as a string).
* `integer` - an integer value (written as a string).
* `datetime` - a date and, potentially, a time.  These are encoded as an ISO string, in the following format: "%Y%m%dT%H%M%S%F%q".  For example, the release date for Chrono Trigger is encoded as "19950311T000000" (no time specified).

Some metadata is also marked as "statistic" - these are kept track of by ES and do not show up in the metadata editor.  They are shown in certain views (for example, the detailed view shows both `playcount` and `lastplayed`).

#### `<game>`

* `name` - string, the displayed name for the game.
* `desc` - string, a description of the game.  Longer descriptions will automatically scroll, so don't worry about size.
* `image` - image_path, the path to an image to display for the game (like box art or a screenshot).
* `thumbnail` - image_path, the path to a smaller image, displayed in image lists like the grid view.  Should be small to ensure quick loading.
* `video` - video_path, the path to a video to display for the game, for themes that support the _video_ viewstyle.
* `rating` - float, the rating for the game, expressed as a floating point number between 0 and 1.  Arbitrary values are fine (ES can display half-stars, quarter-stars, etc).
* `releasedate` - datetime, the date the game was released.  Displayed as date only, time is ignored.
* `developer` - string, the developer for the game.
* `publisher` - string, the publisher for the game.
* `genre` - string, the (primary) genre for the game.
* `players` - integer, the number of players the game supports.
* `playcount` - statistic, integer, the number of times this game has been played.
* `lastplayed` - statistic, datetime, the last date and time this game was played.
* `sortname` - string, used in sorting the gamelist in a system, instead of `name`.


#### `<folder>`
* `name` - string, the displayed name for the folder.
* `desc` - string, the description for the folder.
* `image` - image_path, the path to an image to display for the folder.
* `thumbnail` - image_path, the path to a smaller image to display for the folder.


Things to be Aware Of
=====================

* You can use ES's built-in [scraping](http://en.wikipedia.org/wiki/Web_scraping) tools to avoid creating a gamelist.xml by hand, as described in README.md.

* ES will try to write any image paths as relative to the current system games path or relative to the current home directory if it can.  This is done to try and keep installations portable (so you can copy them between computers).

* One thing to be aware of: the EmulationStation text rendering code doesn't currently support Unicode.  If I fix this in the future, it will probably use UTF-8.  For now, you'll just have to convert names and descriptions to ASCII.  Sorry!

* If a value matches the default for a particular piece of metadata, ES will not write it to the gamelist.xml (for example, if `genre` isn't specified, ES won't write an empty genre tag; if `players` is 1, ES won't write a `players` tag).

* A `game` can actually point to a folder/directory if the the folder has a matching extension.

* `folder` metadata will only be used if a game is found inside of that folder.

* ES will keep entries for games and folders that it can't find the files for.

* The switch `--gamelist-only` can be used to skip automatic searching, and only display games defined in the system's gamelist.xml.

* The switch `--ignore-gamelist` can be used to ignore the gamelist and force ES to use the non-detailed view.

* If at least one game in a system has an image specified, ES will use the detailed view for that system (which displays metadata alongside the game list).

* If you want to write your own scraper, the built-in scraping system is actually pretty extendable if you can get past the ugly function declarations and your instinctual fear of C++.  Check out `src/scrapers/GamesDBScraper.cpp` for an example (it's less than a hundred lines of actual code).  An offline scraper is also possible (though you'll have to subclass `ScraperRequest`).  I hope to write a more complete guide on how to do this in the future.
