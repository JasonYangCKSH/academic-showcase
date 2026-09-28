# JuliaSet 動畫繪製（drawJuliaSet，ARM 組合語言）
 
以 **C 與 ARM 組合語言混合編程**，在 ARM Linux 上計算 Julia Set，並直接寫入 Frame Buffer（`/dev/fb0`）播放動畫。核心繪圖函式 `drawJuliaSet` 完全以 ARM assembly 手寫，全程使用**定點整數運算**，不依賴浮點單元。
 
| 檔案 | 內容 |
|---|---|
| `main.c` | 主程式：呼叫各組語函式、開啟 Frame Buffer、逐幀輸出動畫 |
| `drawJuliaSet.s` | Julia Set 計算與著色（核心） |
| `name.s` | 印出組別與成員姓名 |
| `id.s` | 讀入成員學號、計算總和、等待指令 `p` 後印出 |
| `drawJuliaSet`、`program` | 已編譯的 ARM（armhf）執行檔 |
 
## 演算法概要
 
對畫面上每個像素 `(x, y)` 迭代 `z ← z² + c`，以逃逸前的剩餘迭代次數決定顏色。
 
- **定點數表示**：所有數值放大 1000 倍以整數存放，`c = (-0.7, 0.40 → 0.27)`，逃逸條件 `|z|² > 4` 對應 `zx² + zy² > 4,000,000`。
- **除法最佳化**：乘積後的 `÷1000` 以「乘上倒數常數 `274877907` 再右移 38 位」（`smull` + `asr`）取代除法指令。
- **座標映射**：`zx = 1500·(x − w/2)/(w/2)`、`zy = 1000·(y − h/2)/(h/2)`，其中常數乘法以移位與加減組合完成。
- **著色**：最大迭代 255 次，顏色為 `~(i | i << 8)` 的 16-bit RGB565 值，寫入 640×480 畫面陣列。
- **動畫**：`main.c` 讓 `cY` 從 400 以步長 −5 遞減至 270，每幀重新計算並 `write` 至 Frame Buffer。
此外依課程規範，程式中刻意使用了 Operand2 移位格式、條件執行指令（如 `movlt`、`addgt`）、以 `pc`/`lr` 為運算元的指令等 ARM 特有語法。
 
## 建置與執行
 
需在 ARM Linux 環境（具 `/dev/fb0` 的裝置或模擬器）上編譯：
 
```bash
gcc main.c name.s id.s drawJuliaSet.s -o drawJuliaSet
./drawJuliaSet
```
 
執行流程：印出組員姓名 → 輸入三位成員學號並輸入 `p` → 印出學號與總和 → 再輸入 `p` 開始播放 Julia Set 動畫 → 動畫結束後輸入 `p` 離開。