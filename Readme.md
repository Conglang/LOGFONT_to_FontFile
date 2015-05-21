#LOGFONT to FontFile

**Extract and copy the corresponding OTF/TTF/TTC font file from a LOGFONT struct generated by the user input in a CFontDialog on Windows platform.**

Support most fonts of English and Chinese.

![](https://github.com/Conglang/LOGFONT_to_FontFile/blob/master/demo_pic.png)

**The idea is:**<br>
1. Scan all "font name/file name" matches in registry.
2. Scan font folder path in registry. 
3. Scan all font files(TTF/OTF/TTC) for their other names beside the registered one.
4. Make a dictionary of "font name/file path" matches.
5. When given a LOGFONT, search for its lfFaceName in the dictionary.
6. Use FILE to copy font files to a target folder.

Complemented in a rather simple straight-forward way. For a better structured solution [WATCH THIS](http://fontname.codeplex.com/).

References:<br>
+ http://www.microsoft.com/typography/otspec/
+ http://fontname.codeplex.com/