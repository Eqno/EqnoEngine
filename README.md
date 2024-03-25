### 框架层

- [x] RHI 与引擎层分离，分别由 Game / Render 线程驱动。
- [x] 借助可变参数模板，通过封装实例的创建和销毁，实现 class member / owner 的递归注册。
- [x] 实现异步加载队列，只需将函数 bind 后 push 入队列，便可在单独的 resource 线程执行。
- [x] 引擎层使用 shared_ptr 持有 Object 实例，RHI 层使用 weak_ptr 持有引擎数据。
  - [x] RHI 层借此实现 mesh 的生命周期管理，保证线程安全。
  - [x] 当引擎层销毁 Object 时，其包含的 mesh 将在下一帧开始时销毁。

### RHI 层

- [x] 使用 Z-PrePass 进行深度测试优化，减少 Fragment Shader 的执行次数，提高渲染性能。
- [x] 提供了多套 Lit/Unlit 内置 Shader，其中包括 Blinn-Phong 和基于 Cook-Torrance 的 PBR。
- [x] 使用 Fence、Barrier 和 Semaphore 进行设备、内存、管线、命令和 Render Pass 的同步。
- [x] 支持 Multiple Frames In Flight，可以为交换链指定 Frame 数量，减少 CPU 和 GPU 同步带来的性能浪费。
- [x] 支持 Texture 的 Mipmap 生成，使用线性插值对相邻的 Mipmap 进行采样，防止渲染远处的物体时出现摩尔纹。
- [x] 支持 MSAA 反走样，在 Configs/Graphics 中配置最大采样倍数，自动查询并使用设备支持的、最接近配置的倍数。
- [x] 使用 Shaderc 自动编译 Shader 文件并创建 Shader Stage。
  - [x] 支持 Shader 外定义宏，Shader 内直接使用。
  - [x] 支持 Shader Fallback 功能，在 Material 文件中填写多个 Shader 路径以使用。
  - [x] 支持 Shader Include 的功能，设置 Include Path，使用 File Finder 自动寻找头文件并引入编译。
- [x] 同 Shader Pipeline 合并，减少 DST 和 Pipeline 的创建。
  - [ ] 使用同一 LightChannel 的 mesh Storage Buffer 合并。
  - [x] 使用同一 Camera 或 Material 的 mesh Uniform Buffer 合并。
- [x] 支持 LightChannel，指定多光源作用于 SceneObject。
  - [ ] 多光源着色，光照通道信息使用 Storage Buffer，以动态大小传递给 Fragment Shader，避免 GPU 内存的浪费。
  - [x] 多光源阴影，生成 Shadow Map 时，利用动态 DST 传递光源 VP Matrix，只需在记录 Command Buffer 时换绑。

### 引擎层

- [ ] 重载 operator new 实现 Object 内存池，优化内存分配和销毁的性能消耗。
- [x] 借助 RapidJson 实现简易文件系统，支持 Config 和 Object 模板的序列化和反序列化。
- [x] 使用匿名函数和闭包，返回元素随时可能失效的序列迭代器，动态维护当前场景内的有效灯光。
- [x] 实现基于基向量、与 Unity 类似的 Relative / Absolute Transform 系统。
  - [x] 设置某一 SceneObject 的 Transform 属性时，其他属性也会进行同步。
  - [x] SceneObject 进行 Transform 变换时，子 SceneObject 同时进行递归变换。
- [ ] 使用协程为 RHI 层 ParseMesh 并创建管线，减少面数较多的模型加载时的卡顿。 
- [x] 场景中 Model、Light 和 Camera 等任何 SceneObject 均可在运行时动态添加或删除。
- [x] 实现 InputSystem 输入监听系统，无论在一帧内的何时按下按键，一定在下一帧开始时触发，实现一帧内的监听结果统一。
- [x] 实现 Object 的生命周期管理和事件调用，其中 OnStart 的调用位于下一帧的开始，保证晚于同一帧内的所有 OnCreate。
