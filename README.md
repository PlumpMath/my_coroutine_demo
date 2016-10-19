coroutine demo


* 10-15 15:00

首先我还不清楚实现怎样一个demo去体现协程的特点，暂时想用协程做一个echo server吧

第一步首先恢复下对ucontext的用法，做一个生产者和消费者的模型，然后再在其中添加一个filter，实现状态转换，没错这是PIL中的lua coroutine的典型用法

第二步将其和网络IO调用结合起来，实现一个较高性能的echo server，或者压测工具，然后和epoll模型进行对比；暂时想到这里，begin

* 10-15 18:00

实现了一个最简单的生产者-消费者模型，生产者从STDIN阻塞式读取数据，写入全局buffer，消费者从buffer中读消息；最粗糙之处在于只有producer和consumer各一个，二者互相切换；然后是对于生产者而言，阻塞式的Read只能读取最多buffer大小的数据，然后唤起消费者。

这里设计一个调度中心uctx_main比较好，uctx_main负责调度每个协程，将被调度协程通过数组或链表组织起来，通过协程的状态进行调度；

多协程的话用阻塞IO就会将整个进程挂起，实现网络IO的话选用非阻塞IO；

对于接收多链接的echo server，需要有一个单独的线程listen和accept，阻塞式的；如果不使用epoll的话，确保每个协程响应请求的方式只能依靠调度器的快速调度。

如果使用epoll作为主循环，由事件驱动，与类似nginx的单进程异步处理就没有区别了，协程用在处理事件上恐怕优势不大。

如果不用单独线程去accept然后派发的话，每个协程使用SO_REUSEPORT去监听同一端口，单独作为一个actor，是不错的选择。

* 10-18

用协程实现一个简易的有限状态机 FSM，在[fsm分支里](https://github.com/BG2BKK/my_coroutine_demo/tree/fsm)

