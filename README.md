# C++框架下的JUCE音频插件开发：从制作一个简单Filter开始
## 准备工作：
- 准备好IDE，VS或者Xcode都行，，这里用VisualStudio2022  
- 配置好C++的编译环境，这里用MSVC
- 准备好JUCE的开发库，可以使用Projucer或者Cmake来构建，Projucer由于有相对更加人性化的GUI界面所以更适合新人上手 

[JUCE的Github页面](https://github.com/juce-framework/JUCE)  
[JUCE的官网下载页面](https://juce.com/download/)  

## 配置环境：

在官网上下载Projucer，这是一个免安装的软件，解压到常用路径就可以了

![Projucer的下载](Image\DownloadProjucer.png)

打开可执行文件会弹出一个创建新工程的窗口，这里可以创建多种JUCE工程，既有独立运行的应用，也有各种格式的插件

![新建页面](Image\NewProject.png)

我们就以常用的VST3插件为例，不要忘记勾选juce_dsp的module，一会会用到

![创建VST](Image\CreateVST.png)

值得一提的是，Projucer也有管理解决方案文件的功能，所以一切有关文件层级的修改建议在Projucer中进行，否则可能会导致修改被覆盖  

例如这里我们可以在Projucer中添加额外的module，包括JUCE官方的module和用户自定义的module

![修改文件](Image\ModifyFile.png)

点击File选项卡下点击“Save Project and Open in IDE”

这样会自动唤起VS，可以看到解决方案资源管理器下已经有多个工程，分别是共享代码库，Standalone工程，VST3工程，以及VST3的清单文件

![资源管理器](Image\SolutionAndProject.png)

按F5运行，默认JUCE的构建设置是Standalone，Standalone会构建并执行一个能够独立运行的exe，能够方便地查看UI界面（生成文件也能在构建设置中修改）

但是由于这样并不能直观表现音频插件的DSP处理效果，所以我们还需要将插件插入别的程序中，

![默认生成的界面](Image\DefaultEditor.png)

右键VST的工程并生成，会在Bulid中的对应位置生成VST插件，这个插件就可以挂载在外部软件中了

![构建VST](Image\BuildVST.png)

下图是JUCE的案例工程中自带一个轻量化的主机，可以识别并插入插件，也可以通过修改启动项来在我们的工程构建的过程中自动打开，

![JUCE的主机工程案例](Image\JUCEHost.png)

为了直观起见，这里选择直接挂载在宿主上，这里用的是Bitwig。

![挂载在Bitwig上](Image\BitwigPlugin.png)

可以看到现在空白的VST插件可以正常挂载，默认的Editor上会有一个HelloWorld文本

Bitwig的优势是即使在我们开发过程中插件崩溃，也不会影响宿主运行；而其他宿主可能在会和插件一起崩溃（点名Reaper）

下面就要正式开始代码部分的编写了

## 代码部分：

回到我们的SharedCode，这里有外部依赖，以及一些需要用到的Module，这些Module目前都是JUCE帮我们配置的，后面也能自定义一些自己的Module

![工程的Module](Image\JUCEModules.png)

主要的功能脚本都在工程名下的Source文件夹下，这里已经有4个文件，分别是PluginEditor和PluginProcessor以及各自的.cpp和.h文件

.h和.cpp的分工比较明确，.h可以用来定义类和override一些方法，.cpp来实现方法

PluginEditor主要负责插件页面的设计以及UI交互功能，而PluginProcessor主要负责插件内部的逻辑，对于音频或者midi数据块的处理

![Source文件夹](Image\ProjectSource.png)

当然想要创建更多脚本来管理也是可以的，别忘在Projucer中配置好

PluginProcessor中的类默认继承了`juce::AudioProcessor`；PluginEditor中的类默认继承了`juce::AudioEditor`，所以这两个类都能override一些预先写好的方法

现在我们主要编辑Processor中的内容，这是用来实现音频处理最核心的代码，这里默认已经override了许多方法，但其实这些方法并不复杂，基本上功能都写在名字上了

这里的常用的方法如下

```cpp
//构造函数
MyJUCELearningAudioProcessor();
//解构函数
~MyFilterAudioProcessor()；
//在接受音频信息块前回调
void prepareToPlay (double sampleRate, int samplesPerBlock)；
//在释放资源时回调
void releaseResources()；
//处理音频最主要的函数，在接受音频数据块或midi信息时回调
void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&);
//这个函数返回一个juce::AudioProcessorEditor类的指针，决定了绘制编辑器页面的类
juce::AudioProcessorEditor* createEditor()；
```

```cpp
//在PluginProcessor.cpp中实现方法
juce::AudioProcessorValueTreeState::ParameterLayout MyFilterAudioProcessor::createParameterLayout()
{
    //添加一个有一个参数"LowCut Freq"的Layout
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LowCut Freq",//参数ID
		"LowCut Freq",//参数名称
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),//最小值，最大值，步长，斜率因子
		20.f));//默认值
    //返回Layout
    return layout;
}
```

现在已经有了控制插件的参数，此外还必须要有`juce::dsp::ProcessorChain`对象添加一些效果处理

值得一提的是`juce::dsp::ProcessorChain`对象是可以嵌套的，也就是说一条链中也可以加入其他含有效果的链，而这里只简单地添加了一个`juce::dsp::IIR::Filter<float>`

另外，由于我们往往需要对左右两个声道同时处理，所以需要实例化两个ProcessorChain，让`LeftChain` 和`RightChain`同时做处理

```cpp
//定义两个ProcessorChain对象，包含一个IIR::Filter<float>对象
juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> LeftChain;
juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> RightChain;
```
我们跳转到PluginProcessor.cpp中，是时候直接处理音频了，不过在此之前还有一件事，将`createEditor()`中的返回值更改为`juce::GenericAudioProcessorEditor(*this)`

原来默认的返回值`MyFilterAudioProcessorEditor (*this)`是在PluginEditor中编辑的界面，也就是一开始我们看到的空白页面

而`GenericAudioProcessorEditor(*this)`会直接将apvts中的参数分配到界面上，这样就可以省去创建Slider的步骤了

当然如果想要一个更加精致的UI页面，也可以自己编辑Editor，JUCE官方也有完整的GUI教程

```cpp
juce::AudioProcessorEditor* MyFilterAudioProcessor::createEditor()
{
    //return new MyFilterAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}
```
来到`processBlock`方法下，这里是音频处理的核心方法，在创建工程时已经有了几句代码，解释如下：

```cpp
//禁用浮点数的非规格化处理，提高处理效率
juce::ScopedNoDenormals noDenormals;
//获取当前输入输出的通道数
auto totalNumInputChannels  = getTotalNumInputChannels();
auto totalNumOutputChannels = getTotalNumOutputChannels();
//清除多余的输出通道，防止啸叫
for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear (i, 0, buffer.getNumSamples());
//循环获取每个通道的音频数据块，并作处理
for (int channel = 0; channel < totalNumInputChannels; ++channel)
{
    auto* channelData = buffer.getWritePointer (channel);
    //这里可以处理每次循环中获取的数据块，在这里可以写相关处理函数
}
```
这个方法在每次获取音频数据块的时候调用，而这个音频数据块可以理解为音频处理中的“一帧”，如果块越小，延迟越小，CPU的负载越大；块越大则反之

Bitwig中在下图选项中设置数据块大小，其他DAW中也会有相似的选项：

![设置数据块](Image\AudioBlock.png)

虽然在以上代码中已经提供了可以处理的数据块，但是由于我们要实现的是一个Filter，根据我们小学二年级学过的DSP，‌IIR滤波器的结果不仅与当前的输入信号有关，还与过去的输入和输出信号有关

这里获取的数据块是当前音频处理的单个离散采样块，所以这对一个滤波器来说是肯定不够的，不过好在JUCE已经对我们提供了名为`juce::dsp::ProcessContextReplacing`的类，用于获取上下文的输入信息

所以我们只需要让我们的`ProcessorChain`处理当前`ProcessContextReplacing`的实例就好了，下面就让我们获取这个实例

```cpp
//这里用于获取apvts的参数LowCut Freq，由于getRawParameterValue()获取的是一个指针，所以别忘记加上->load()
float lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
//get<0>是为了获取ProcessorChain的第一个内容，也就是滤波器，coefficients用于设置滤波器的系数
LeftChain.get<0>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), lowCutFreq);
RightChain.get<0>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), lowCutFreq);
//这里获取声音数据块，注意声音数据块也是分通道的，用getSingleChannelBlock()方法拆解为左右声道
juce::dsp::AudioBlock<float> block(buffer);
auto leftBlock = block.getSingleChannelBlock(0);
auto rightBlock = block.getSingleChannelBlock(1);
//利用上面获取的当前声音数据块来获取声音上文信息
juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
```
到这里基本上就大功告成了，我们既有了“效果器链”，也有了要处理的“声音信息”

下面只需要一个简单的函数调用就可以让我们的Filter工作起来
```cpp
LeftChain.process(leftContext);
RightChain.process(rightContext);
```

现在重新生成VST插件，在宿主中挂载，听听效果

![大功告成](Image\MyFilter.png)

不出意外的话滑块已经能实时控制这个低切滤波器的截止频率，而宿主可以读取到这个插件中的参数，你可以像对其它插件一样对这个Filter设计自动化

不过现在这个Filter还很简陋，只有一个截止频率参数，你也可以用各种JUCEAPI设计高低切的坡度（Slope），多设计几个频点，以及设计波表图和滤波曲线

如果JUCE原本的库不能满足你的需要，甚至你可以编写自己的库，去拓展更多功能！