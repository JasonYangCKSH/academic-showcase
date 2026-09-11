# Dual-Chef Order Scheduling Simulator (FIFO vs. SQF)

以佇列（Queue）模擬餐廳點餐系統，比較**單一廚師 FIFO** 與 **雙廚師 SQF(Shortest Queue First)** 兩種排程策略，在訂單湧入情境下的服務效率與失敗率（逾時、爆單）表現。

## 核心邏輯

每筆訂單具備到達時間（Arrival）、烹調時長（Duration）、逾時上限（TimeOut）。系統依序處理訂單，並依下列規則判定成功、逾時或放棄：

- **佇列已滿（容量 3）**：新訂單直接記入 Abort List
- **廚師處理超過訂單的 TimeOut**：記入 Timeout List 並計算延遲時間
- **合法完成**：正常出餐

## 兩種排程策略

- **單廚師 FIFO**（`Command2`）：所有訂單依到達順序進入單一佇列，逐一處理。
- **雙廚師 SQF**（`Command3`）：新訂單依序判斷——優先分配給**閒置中**的廚師；若兩者皆忙碌，則分配給**佇列較短**者；佇列長度相同時以較小 CID 為準；若兩廚師佇列皆滿，則訂單放棄（Abort）。

## 輸出統計

- Abort List / Timeout List（含每筆訂單延遲時間）
- Total Delay（總延遲分鐘數）
- Failure Percentage（逾時+放棄訂單占「理論上可準時完成」訂單之比例）

## 使用技術

C++（Shell Sort 依到達時間排序輸入資料，`vector` 實作佇列與訂單清單管理）