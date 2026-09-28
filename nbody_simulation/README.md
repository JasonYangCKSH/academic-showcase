# 異質粒子場景下 Octree 與 Verlet List 之碰撞偵測

![Uniform Grid 與 Octree 碰撞偵測模擬畫面](image.png)

大學專題：在**不改變碰撞判定正確性**的前提下，比較 broad-phase 空間分割結構（Uniform Grid、Octree）結合 Verlet List 自適應 skin 機制，對重建次數、候選配對數與執行效能的影響。

> **對應論文**：楊耀甯、吳宜鴻，〈異質動態粒子場景下整合八元樹空間分割與 Verlet List 自適應更新之碰撞偵測〉，TANET 2026 臺灣網際網路研討會（投稿審查中）。
> 以下「論文式（n）」「論文表 n」皆指此論文。

| 資料夾 | 內容 |
|---|---|
| [`collision/`](#collision) | 核心演算法、benchmark 與正確性測試（C++17） |
| [`system/`](#system) | 以 `collision/` 為基礎的可視化展示系統（TypeScript） |

---

## collision

> 粒子運動（邊界反彈 + 隨機初速度）僅用於產生碰撞測試資料，不涉及重力或 N-body 力學模擬。

### 目錄結構

```
collision/
├── include/
│   ├── particle.h            # 粒子資料結構
│   ├── broad_phase.h         # UniformGrid、Octree
│   ├── narrow_phase.h        # 球體重疊判定
│   ├── brute_force.h         # O(n²) 暴力法（正確性基準）
│   ├── verlet_buffer.h       # skin 計算、上界限制、重建判斷
│   ├── collision_response.h  # 彈性碰撞衝量、邊界反彈
│   ├── scenario.h            # 測試場景產生器
│   ├── simulation.h / .cpp   # 模擬主迴圈
│   └── glm/                  # 向量數學函式庫（已內附）
├── bench/                    # Benchmark 進入點與掃描邏輯
├── test/                     # 正確性比對測試
└── info/benchmark*.csv       # 論文實驗之原始結果（見下方對照表）
```

### 演算法概要

**兩階段偵測**
- **Broad-phase**：以空間分割結構篩出候選配對，避免 O(n²) 兩兩比較。
- **Narrow-phase**：以 `(r_a + r_b)² ≥ |pos_a − pos_b|²` 精確判定，使用平方距離省去開根號。

**Uniform Grid**
- 以 hash map（`int64_t` 格子鍵值 → 粒子索引）實作稀疏格子，不需預先配置整個空間。
- 鄰居搜尋只檢查格內配對與 **13 個前向鄰格**，每組配對只檢查一次，輸出統一為 `(小, 大)`。

**Octree**
- 葉節點超過 `leafCapacity` 時分裂為八，並以 `maxDepth` 限制深度，避免粒子高度重疊時無限遞迴。
- 葉節點之間先以包圍盒（加上半徑與 skin 的延伸範圍）快速剔除，再逐一比對粒子。

**Verlet List 與自適應 skin（本研究核心）**

每個粒子在半徑外維護一層緩衝區 skin。只要所有粒子自上次重建以來的累積位移都未超過自身 skin，就沿用既有的候選配對、跳過 broad-phase 重建。

| 項目 | 說明 | 對應論文 |
|---|---|---|
| skin 計算 | `skin = K·\|v\|·Δt + ½·\|a\|·(K·Δt)²`，依粒子瞬時速度與加速度計算，**只在重建時計算一次**（正確性前提） | 式（1） |
| 重建判定 | 任一粒子累積位移 `Δx = ‖x − x_last‖ > skin` 即觸發全域重建 | 式（2） |
| Uniform Grid 上界 | `max(0, cellSize/2 − radius)`，確保候選列表不漏抓 | 式（3）、3.4 節 |
| Octree 上界 | `max(0, 所屬葉節點半邊長 − radius)`，避免密集區候選數過度膨脹 | 式（4）、3.4 節 |
| 候選判定 | 以 `(r_a + skin_a) + (r_b + skin_b)` 作為候選距離，同時涵蓋兩粒子各自的 skin | 3.5 節 |
| K 值 | 預測跨步係數；K 愈大 → skin 愈厚、重建愈少，但候選配對數上升 | 3.3 節 |

### 模擬流程（`Simulation::step()`）

1. **BruteForce**：每幀 O(n²) 全配對，作為正確性基準。
2. **UniformGrid / Octree**：第一幀或 skin 失效時重建結構、更新 skin 與位置快照，再對候選配對做 narrow-phase。
3. **碰撞回應**：彈性碰撞衝量 + 依質量反比做位置修正（避免粒子黏住）、世界邊界反彈。
4. 每幀記錄 broad／narrow／response 耗時、是否重建、候選配對數與碰撞數。

測試場景（固定 seed，可重現）：`uniformCloud`、`explosion`、`spatialCluster`（可調聚集比例、熱點數與高速粒子比例，benchmark 預設使用）。

### Benchmark

實驗設定與論文表 1 一致：粒子數 10000、總時間步數 1000、單執行緒量測。

1. 以 BruteForce 產生每幀碰撞配對作為正確性基準。
2. 執行無 skin 的 `uniform_grid`、`octree`（每步重建）作為基準。
3. 對 K ∈ {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000} 執行 `uniform_grid_skin`、`octree_skin`，每組重複 10 次，取平均與標準差。

輸出欄位：總時間與各階段時間（平均／標準差）、`rebuild_count`、`avg_candidate_per_rebuild`、`correctness_ok`、`first_mismatch_frame`、`repeat_count`。

#### 結果檔案與論文場景對照

各檔案的 `scenario` 欄位皆為 `spatial_cluster`（同一產生器、不同參數），對應關係如下：

| 檔案 | 論文場景 |
|---|---|
| `info/benchmark.csv` | 均勻／低速差 |
| `info/benchmark2.csv` | 群聚／低速差 |
| `info/benchmark3.csv` | 均勻／高速差 |
| `info/benchmark4.csv` | 群聚／高速差 |

#### K = 100 結果摘要（對應論文表 2、表 3）

| 場景 | 重建次數 UG | 重建次數 OT | 平均候選數 UG | 平均候選數 OT | 總執行時間 UG (s) | 總執行時間 OT (s) |
|---|---|---|---|---|---|---|
| 均勻／低速差 | 70 | 28 | 56,752 | 343,152 | 0.413 | 1.144 |
| 群聚／低速差 | 258 | 194 | 293,765 | 532,434 | 2.144 | 4.551 |
| 均勻／高速差 | 1000 | 500 | 57,556 | 370,755 | 3.826 | 10.203 |
| 群聚／高速差 | 1000 | 513 | 70,426 | 391,848 | 4.003 | 11.148 |

UG 為 Uniform Grid、OT 為 Octree；總執行時間為 1000 個時間步之 broad-phase、narrow-phase 與碰撞回應時間累計（不含場景初始化），取 10 次平均。四種場景於所有 K 值下，`correctness_ok` 皆為 1（與暴力法完全一致）。完整數據（含無 skin 基準與各階段時間）見 `info/`。

### 正確性測試

`test/test_correctness.cpp`：逐幀比對 BruteForce 與 UniformGrid（開啟 skin）的碰撞配對是否完全一致。

### 建置與執行

需求：CMake ≥ 3.14、支援 C++17 的編譯器（`glm` 已內附，無其他相依套件）。

```bash
cd collision && mkdir -p build && cd build
cmake .. && cmake --build .

./bench              # 輸出 benchmark.csv
./test_correctness   # 輸出 PASS / FAIL
```

---

## system

以 `collision/` 為基礎的互動式可視化系統，用於展示 Brute Force、Uniform Grid、Octree 與 Verlet skin 機制的運作過程。

技術：TypeScript、React、React Three Fiber、Vite、Zustand。

```bash
cd system
npm install
npm run dev     # 啟動開發伺服器
npm test        # 執行核心邏輯單元測試（Vitest）
npm run build   # 產生正式版
```