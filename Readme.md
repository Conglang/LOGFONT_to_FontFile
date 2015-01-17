#LOGFONT to FontFile
---

**Analyse a font file's path from Win32 LOGFONT structure and copy the OTF/TTF/TTC file to target folder.**

Chinese Traditional and Simplified fonts supported.

Application looks like this:

![](https://github.com/Conglang/LOGFONT_to_FontFile/blob/master/demo_pic.png)

**The idea is:**

1. Scan all "font name/file name" matches in registry.
2. Scan font folder path in registry. 
3. Scan all font files(TTF/OTF/TTC) for their other names beside the registered one.
4. Make a dictionary of "font name/file path" matches.
5. When given a LOGFONT, search for its lfFaceName in the dictionary.
6. Use FILE to copy font files to a target folder.

Complemented in a rather simple straight-forward way. Watch [THIS](http://fontname.codeplex.com/) for a better structured solution.

References:

+ http://www.microsoft.com/typography/otspec/
+ http://fontname.codeplex.com/