# MiniTP

一个轻量的 C++ 线程池，使用固定数量的工作线程执行任务。

## 特性

- 支持提交无返回值任务
- 支持提交带返回值的任务，并通过 `std::future` 获取结果
- 支持等待已经提交的任务全部完成
- 线程池销毁时会先执行队列中剩余的任务，再等待工作线程退出

目前不支持运行时动态调整线程数量。如需更改线程数量，请销毁当前线程池后重新创建。

## 环境要求

- CMake 4.4 或更高版本
- 支持 C++23 的编译器

## 集成

将 `MiniTP` 放到项目目录中，在项目根目录的 `CMakeLists.txt` 中添加：

```cmake
add_subdirectory(MiniTP)
target_link_libraries(main PRIVATE MiniTP)
```

其中 `main` 是使用线程池的目标名称。然后在 C++ 代码中包含头文件：

```cpp
#include <MiniTP/ThreadPool.hpp>
```

## API

### 创建线程池

```cpp
MiniTP::ThreadPool pool(4); // 创建 4 个工作线程
```

构造函数要求线程数量大于 0，否则会抛出 `std::runtime_error`。

### `run`

提交一个无返回值任务。函数和参数会被复制或移动到任务中。

```cpp
pool.run([](int value) {
	// 处理任务
}, 42);
```

### `runWithReturn`

提交一个带返回值的任务。由于返回类型位于模板参数中，需要显式指定返回类型：

```cpp
std::future<int> result = pool.runWithReturn<int>([](int left, int right) {
	return left + right;
}, 20, 22);

int value = result.get(); // value == 42；get() 会等待任务完成
```

如果任务抛出异常，异常会由 `future::get()` 重新抛出。

### `waitAll`

等待当前已经提交的任务全部完成：

```cpp
pool.run([] { /* task 1 */ });
pool.run([] { /* task 2 */ });
pool.waitAll();
```

建议调用`waitAll`的时候不要同时提交任务

## 完整示例

```cpp
#include <MiniTP/ThreadPool.hpp>
#include <iostream>

int main()
{
	MiniTP::ThreadPool pool(4);

	for (int i = 0; i < 8; ++i) {
		pool.run([i] {
			std::cout << "task " << i << '\n';
		});
	}

	auto result = pool.runWithReturn<int>([](int value) {
		return value * value;
	}, 6);

	pool.waitAll();
	std::cout << "result: " << result.get() << '\n';
}
```

任务执行期间抛出的异常不会让工作线程退出；无返回值任务的异常会输出到标准错误流，带返回值任务的异常则通过 `future::get()` 获取。