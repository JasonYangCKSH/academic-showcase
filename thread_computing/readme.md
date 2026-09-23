# Parallel Sorting: BubbleSort + MergeSort with Multi-Process / Multi-Thread

比較單一流程、多進程（multi-process）、多執行緒（multi-thread）三種策略對大量資料進行 BubbleSort + MergeSort 的效能，並實測驗證理論時間複雜度與實際開銷之間的落差。

## 1. 實作方法

| # | 方法 | 做法 | 理論複雜度 |
|---|---|---|---|
| 1 | Baseline | 整筆資料直接 BubbleSort | $`O(N^2)`$ |
| 2 | 單一 process 切片 | 切成 K 份後於同一 process 逐一 BubbleSort，再以遞迴 TreeMerge 合併 | $`O\left(\frac{N^2}{K} + N\log K\right)`$ |
| 3 | Multi-process | `mmap()` 建立 shared memory，`fork()` 產生 K 個 child process 各自排序，再以 K−1 個 process 進行 TreeMerge；以 `waitpid()` 回收，避免 zombie process | $`O\left(\frac{N^2}{K^2} + N\log K\right)`$ |
| 4 | Multi-thread | `<thread>` 建立 K 個 thread 各自排序，`join()` 回收後再以 thread 進行 TreeMerge；thread 共享 data section，不需 shared memory | $`O\left(\frac{N^2}{K^2} + N\log K\right)`$ |

<center>

![TreeMerge 示意圖](image-1.png)

**圖 1**　TreeMerge 示意圖

</center>

## 2. 實驗結果

<center>

![四種方法的執行效能比較圖](image.png)

**圖 2**　四種方法的執行效能比較（以 $`K=20`$ 為例）

</center>

## 3. 關鍵發現

| 發現 | 說明 |
|---|---|
| K 越大不代表越快 | K 夠大時，process／thread 的建立與回收開銷反而主導總時間，偏離理論複雜度；例如 K = 5000、10000 時，方法 3、4 的執行時間不減反增 |
| 切太細後與 N 脫鉤 | 切片小到 BubbleSort 幾乎不耗時，執行時間便完全由建立、context switch 與同步等待（`waitpid()`／`join()`）主導，不同 N 的執行時間趨於一致 |
| Process 開銷 > Thread 開銷 | 相同 K 下，方法 3 的執行時間持續高於方法 4，因建立 process 的成本高於 thread |
| 平行化有效益門檻 | N 與 K 皆夠大時優勢才會顯現（例如 N = 500K、1M 時，方法 3、4 明顯優於方法 2）；資料量不足時，管理成本反而拖慢整體效能 |

## 4. 開發環境與使用技術

| 項目 | 內容 |
|---|---|
| 環境 | Windows + WSL2（Debian）、g++ 14.2.0 |
| 語言 | C++ |
| 系統呼叫／函式庫 | `<sys/mman.h>`（shared memory）、`<unistd.h>`（`fork`／`waitpid`）、`<thread>` |