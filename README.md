Cocoa: 2D Rendering Engine
==========================
**************************

Introduction
------------
Cocoa is a 2D rendering engine written in C++. It provides a simple and quick way to create
your own 2D interface by using CSS, XML and Javascript (Typescript is optional).

Cocoa can be used in many situations such as game, user interface or something else.

Architecture
------------
Cocoa consists of different modules:
* **Core** provides basic functions for other parts.
* **Ciallo** is a powerful and hardware-accelerated 2D rendering engine based on Skia.
* **Komorebi** parses XML and CSS file to a DOM tree, and runs Javascript.
* **MaidCafe** implements a "standard library" for Javascript. It allows Javascript
  to read/write files, access network, play audio and so on.
* **Reactor** is a JIT compiler based on LLVM.

There are also some standalone tools that help you develop with Cocoa:
* **Inspector** can load XML and CSS file then analyze the layout and style information.
* **Viewer** can analyze XML and Javascript file dynamically.

XML Syntax
----------
An XML file specifies the what should be drawn. Here's an "Hello World" example:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <property name="signature" type="string">org.CocoaProject.Layout</property>
    <widget class="Window">
        <property name="width" type="integer">400</property>
        <property name="height" type="integer">400</property>
        <property name="title" type="string">Hello World</property>
        <widget class="TextLabel" id="dialog-box">
            <property name="content" type="string">Hello, World!</property>
        </widget>
    </widget>
</interface>
```

A simple visual novel interface:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <property name="signature" type="string">org.CocoaProject.Layout</property>
    <property name="stylesheet" type="file">res/visual_novel_example.css</property>
    <widget class="Window">
        <property name="width" type="integer">1280</property>
        <property name="height" type="integer">720</property>
        <property name="title" type="string">Cocoa Visual Novel Sample</property>
        <property name="icon" type="file">res/icon/visual_novel_sample.jpg</property>
        <emit signal="window-close" event="WindowCloseEvent"/>

        <widget class="ImageFrame" id="background-image"/>
        <widget class="ImageFrame" id="character-stand-image"/>
        <widget class="TextLabel" id="chapter-label"/>
        <widget class="Container" id="bottom-container">
            <widget class="TextLabel" id="character-name-label"/>
            <widget class="ImageFrame" id="character-face-image"/>
            <widget class="KaleidoscopicText" id="dialog-content">
                <property name="KTDLSupport" type="boolean">True</property>
            </widget>
            <widget class="Container" id="dialog-buttons">
                <widget class="Button" id="save-button">
                    <property name="icon" type="file">res/icon/save_button.jpg</property>
                    <property name="text" type="string">Save</property>
                    <emit signal="save-button-clicked" event="ClickEvent"/>
                </widget>
                <widget class="Button" id="load-button">
                    <property name="icon" type="file">res/icon/load_button.jpg</property>
                    <property name="text" type="string">Load</property>
                    <emit signal="load-button-clicked" event="ClickEvent"/>
                </widget>
                <widget class="Button" id="auto-button">
                    <property name="icon" type="file">res/icon/auto_button.jpg</property>
                    <property name="text" type="string">Auto</property>
                    <emit signal="save-button-clicked" event="ClickEvent"/>
                </widget>
                <widget class="Button" id="settings-button">
                    <property name="icon" type="file">res/icon/settings_button.jpg</property>
                    <property name="text" type="string">Settings</property>
                    <emit signal="save-button-clicked" event="ClickEvent"/>
                </widget>
                <widget class="Button" id="exit-button">
                    <property name="icon" type="file">res/icon/exit_button.jpg</property>
                    <property name="text" type="string">Exit</property>
                    <emit signal="exit-button-clicked" event="ClickEvent"/>
                </widget>
            </widget>
        </widget>
    </widget>
</interface>
```
See *docs/* directory for more detailed documentations.

Javascript and MaidCafe
-----------------------
Cocoa's Javascript engine is based on Google's V8. But MaidCafe provides many powerful function
like Node.js. It can manipulate DOM tree and its styles.
Here's a simple example by using MaidCafe.
```javascript
try {
    let scenario = MaidCafe.open("scenario.txt").read();
    let widget = MaidCafe.document.getWidgetById("dialog-box");
    widget.content = scenario.toUtf8String();
} catch (e) {
    MaidCafe.print(e);
}
```
See *docs/* directory for more detailed documentations.

More effects
------------
Cocoa supports many interesting and beautiful visual effects. They may cause some performance
problems while they can improve the appearance of your application.

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

