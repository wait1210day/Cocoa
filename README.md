Cocoa: 2D Rendering Engine
==========================
**************************

Introduction
------------
Cocoa is a 2D rendering engine written in C++. It provides a simple and quick way to create
your own 2D interface by using Javascript (Typescript is optional).

Cocoa can be used in many situations such as game, user interface or something else.

Architecture
------------
Cocoa consists of different modules:
* **Core** provides basic functions for other parts.
* **Ciallo** is a powerful and hardware-accelerated 2D rendering engine based on Skia.
* **Komorebi** implements many widget in C++, and runs Javascript.
* **MaidCafe** implements a "standard library" for Javascript. It allows Javascript
  to read/write files, access network, play audio and so on. It compatible with Node.js.
* **Reactor** is a JIT compiler based on LLVM.

Javascript in Cocoa
-----------------------
Cocoa's Javascript engine is totally compatible with Node.js.
See *docs/* directory for more detailed documentations.

Animations
----------
Cocoa can load a Lottie JSON file for playing animations. Lottie files can be exported by
*Adobe After Effect* or other similar tools.

Komorebi Extensions
-------------------
Cocoa supports many interesting and beautiful visual effects. They're provided as extensions of
**Komorebi**.

To load an extension in Javascript:
```javascript
let live2d = window.require("CubismLive2DWidget");
```

### KaleidoscopicText Widget
For text rendering, **Kaleidoscopic Text Description Language** or **KTDL** can be used.
KTDL is a markup language that focuses on the display and layout of text.
KTDL allows the user to specify a series of attributes for a text that will be applied
to the text within its scope.

Following chart shows some useful attributes in KTDL.

|     Attribute/Declaration        |    Shorthand     | Description |
|----------------------------------|------------------|-------------|
| [n!]                             | [n!]             | New line |
| [t!]                             | [t!]             | Tab character (8 spaces is the default) |
| [set-tab *integer*]              | [stab *integer*] | Set spaces for a tab character |
| [font-size *integer*]            | [ftsz *integer*] | Specify font size (pixels) |
| [font-style Normal,Italic,Bold]  | [ftst N,I,B]     | Specify font style |
| [font-weight Normal,Bold,Light]  | [ftw N,B,L]      | Specify font weight |
| [font-name *string*]             | [fn *string*]    | Specify font name |
| [foreground *hex color*]         | [fg *hex color*] | Specify foreground color (font color) |
| [background *hex color*]         | [bg *hex color*] | Specify background color |
| [link *URI*]                     | [link *URL*]     | Hyperlink |
| [translate *content*]            | [trl *content*]  | Display the translation above the text |
| [black-mask]                     | [bmask]          | A black opaque mask appears on the text |
| [underline]                      | [ul]             | Render underline |
| [strikeout]                      | [sto]            | Render strikeout |

The following example uses KTDL to display the corresponding Hiragana on Japanese Kanji characters.
<pre style="font-family: Consolas, sans-serif; font-size: 14px">
[ftsz 15]
[trl みらい]未来[!trl]の[trl まえ]前[!trl]にすくむ[trl てあし]手足[!trl]は[n!]
[trl しず]静[!trl]かな[trl こえ]声[!trl]にほどかれて[n!]
[trl さけ]叫[!trl]びたいほど　なつかしいのは[n!]
ひとつのいのち[n!]
[trl まなつ]真夏[!trl]の[trl ひかり]光[!trl][n!]
あなたの[trl かたに]肩[!trl]に　[trl ゆれ]揺れ[!trl]てた[trl こもれび]木漏れ日[!trl][n!]
[!ftn]
</pre>

### Cubism Live2D Widget
TODO: Complete this.
