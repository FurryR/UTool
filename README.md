# UTool main

此 repo 可能落后于 `@ookamitai` 的 fork。

## 特性

- `:play` 功能 (未完全同步)
- `.ust` 文件编辑操作 (未完全同步)
- 插件系统 (未同步)
- 命令系统 (未完全同步)
- 一些其它的特性？

## 使用

要编译该程序，请在 **Windows** 下使用以下命令：

```powershell
make build
# make debug # 调试的情况
```

然后使用以下命令打开程序：

```powershell
./utool
```

你可以使用以下命令打开一个 `.ust` 文件：

```bash
:load [filename]
```

然后，请使用 **方向键** 移动，并且在需要的单元格上按 **i** 键来修改文件。

当您需要插入音符的时候，您可以按下 **n** 键或 **m** 键，这样会在当前音符的 **前面** 或 **后面** 插入一个休止符。

当然，如果您希望不同的表示方式，您可以在 **NoteNum** 或者 **Length** 上按 **t** 键，这样就可以切换表示方式。(请注意，在 **NoteNum** 单元格上按 **t** 键同样也会影响到编辑！)

当编辑完成后，您可以使用以下方式保存文件：

```bash
:save (filename) # filename 为可选。当创建新工程时必须指定。不指定时将保存到当前文件。
```

然后使用以下两个之中的一个命令来关闭编辑器：

```bash
:close # 关闭工程，不关闭编辑器
:q     # 退出
```

您可以使用 **play** 命令来预览您的工程(MIDI 播放)：

```bash
:play (end) # end 不指定的时候默认为播放到末尾。
```

您可以使用 **plugin** 命令来加载插件：

```bash
:plugin # 打开 Plugin SubUI
```

当您打开插件 UI 之后，您可以选择加载(Enter)或卸载插件(Backspace)。

在按下 **Enter** 键之后，您可以选择手动输入插件的路径，或直接拖入插件 DLL，然后按下回车。

此时，您将可以看到插件的名字(形如 `Loaded module 'owo'`)，此时插件已经被加载。您可以使用插件的名字进行对插件的调用。

比如，我们假设有一个插件 `owo`，其中有功能 `vcv2cv`，则命令调用形式如下：

```bash
:owo vcv2cv (更多参数...) # 在这里，vcv2cv 不接受任何参数，但我们仍可以指定一些。
```

## 插件开发

### 注意

由于各个编译器的 C++ ABI 不兼容，故编译插件的编译器和编译主程序的编译器最好一致，否则可能发生无法加载的问题。

您可以在 `example/plugin.cpp` 找到一个插件的示例。

以下是通常编译插件的方法：

```powershell
g++ -shared [file].cpp -o [output].dll
```

请酌情添加任何不会导致不兼容的参数。

您的插件应当包含一个名为 `_export` 的导出函数。以下是 `_export` 函数的声明：

```cpp
/**
 * @brief 导出函数。
 * @return std::pair<std::string, Parser> first = 插件的名字(调用用) second = Parser(命令管理器)
**/
extern "C" std::pair<std::string, Parser> _export();
```

> > > 为什么要这么麻烦？直接从参数传入 Parser\* 不更好么？

实际上，插件系统是隔离的，也即您无法访问任何除您插件以外的命令，这样提高了安全性。

事实上，您的插件也不应当通过命令的方式依赖其它插件。

您可以通过以下方式注册一个命令：

```cpp
Parser p;
p.set("command_name", [](const std::string& args, Parser* parser, UI* ui, Editor* editor) -> bool {
  // args(参数)：如果调用命令为`:example_plugin command_name args`，则 args 将为 `args`。
  // parser：插件命令管理器自身。
  /*
    ui：用于绘制界面。
    通常，您无需关心 `render_bar` 和 `render_note`；您可以使用 `render_log` 来在底栏显示 1 行(带颜色的)消息。
    `ui->render_log` 将需要 std::vector<Character>，其可以由 ColorText(...).output() 得到。Character 提供了更细粒度的色彩控制。
    如果您只希望显示一种颜色，您可以使用 ColorText("文本", "颜色代码(不可见)").output() 来立刻得到 std::vector<Character>。
    您也可以拼接多个由 ColorText 得到的 std::vector<Character> 来实现使用多个 ColorText 显示不同的颜色，这也是一种不错的选择。
    然而，UI 在 `ui->render_log` 之后不会立刻更新，您需要调用 `ui->update` 才可以更新 UI。请尽可能少调用 `ui->update`，因为滥用 update 将导致显著的闪屏。
  */
  /*
    editor：用于操作项目。
    通常，`editor->count` 代表当前的光标位置，`editor->column` 代表选中的列(0-4)，这样您可以控制光标。
    您可以通过 `editor->project()` 获得项目的只读引用，而若希望操作引用，则需要 `editor->action` 或 `editor->action_norec`。
    这两个 API 都可以完成编辑操作，区别是后者无法进行撤销。
    请参照示例项目使用 Action / ActionType。
  */
  return false; // 返回后是否进行对 UI 的刷新。如果需要显示一些消息，请返回 false。
  // 您可以使用 editor->render(ui) 手动刷新 UI。
});
p.set_default([](const std::string&, Parser*, UI* ui, Editor*) -> bool {
  // 和上面相同，区别在于这是一个 fallback 函数，当找不到命令时才会调用。
  // args 的区别：:example_plugin nonexist args，则 args 为 nonexist args。
  // 其余完全相同。适合做命令拼写错误时的提示，当然也可以用于别的。
  return false;
});
```

## 鸣谢

- @ookamitai (企划/大部分开发)
- @VeroFess (调试指导)
- @FurryR (重构担当/一小部分的开发)

此外，UTool 也参考(使用)了以下仓库的(部分)源代码：

- [Awacorn](https://github.com/FurryR/Awacorn) - 事件调度库(play 功能)
- [lightpad](https://github.com/FurryR/lightpad) - TUI C++ 11 编辑器(Screen API、UI 布局、设计模式)

感谢以上用户及仓库做出的贡献。
