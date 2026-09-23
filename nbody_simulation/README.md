# 碰撞偵測方法比較

![Uniform Grid 與 Octree 碰撞偵測模擬畫面](image.png)

大學專題：在**不改變碰撞判定正確性**的前提下，比較不同 broad-phase 空間分割結構（Uniform Grid、Octree）與 Verlet buffer 機制對重建次數及整體效能的影響。

| 資料夾 | 內容 |
|---|---|
| [`collision/`](#collision) | 核心演算法、benchmark 與正確性測試（C++17） |
| [`system/`](#system) | 以 `collision/` 為基礎的可視化模擬展示系統(typescript) |

---

## collision

> 粒子運動（邊界反彈 + 隨機初速度）僅用於產生碰撞測試資料，不涉及重力／N-body 模擬。

### 目錄結構

```
collision/
├── include/
│   ├── particle.h            # 粒子資料結構
│   ├── broad_phase.h         # UniformGrid、Octree
│   ├── narrow_phase.h        # 球體重疊判定
│   ├── brute_force.h         # O(n²) 暴力法（ground truth）
│   ├── verlet_buffer.h       # skin 計算、上限裁切、rebuild 判斷
│   ├── collision_response.h  # 彈性碰撞衝量、邊界反彈
│   ├── scenario.h            # 測試場景產生器
│   ├── simulation.h / .cpp   # 模擬主迴圈
│   └── glm/                  # 向量數學函式庫（已內附）
├── bench/                    # Benchmark 進入點與掃描邏輯
├── test/                     # 正確性比對測試
└── info/benchmark*.csv       # 已產生的實驗結果
```

### 演算法概要

**兩階段偵測**
- **Broad-phase**：以空間分割結構篩出候選配對，避免 O(n²) 兩兩比較。
- **Narrow-phase**：以 `(r_a + r_b)² ≥ |pos_a − pos_b|²` 精確判定，使用平方距離省去開根號。

**Uniform Grid**
- 以 hash map（`int64_t` 格子鍵值 → 粒子索引）實作稀疏格子，不需預先配置整個世界。
- 鄰居搜尋只檢查格內配對與 **13 個前向鄰格**，每組配對只會被檢查一次，輸出統一為 `(小, 大)`。

**Octree**
- 葉節點超過 `leafCapacity` 時分裂為八，並以 `maxDepth` 限制深度，避免粒子高度重疊時無窮遞迴。
- 葉節點之間先以包圍盒（加上半徑與 skin 的 margin）快速剔除，再逐一比對粒子。

**Verlet Buffer（本研究核心）**

每個粒子在半徑外維護一層緩衝殼 skin，只要所有粒子自上次重建以來的位移都未超過自身 skin，就沿用快取的候選配對、跳過 broad-phase 重建（對應論文 Condition 5）。

| 項目 | 說明 |
|---|---|
| skin 計算 | `skin = K·\|v\|·dt + 0.5·K²·\|a\|·dt²`，**只在重建時計算一次**（正確性前提） |
| Uniform Grid 上限 | `cellSize/2 − radius`（需考慮兩粒子的延伸半徑相加） |
| Octree 上限 | 所屬葉節點的 `halfExtent − radius` |
| K 值取捨 | K 愈大 → skin 愈厚、重建愈少，但候選配對數上升 |

### 模擬流程（`Simulation::step()`）

1. **BruteForce**：每幀 O(n²) 全配對，作為正確性基準。
2. **UniformGrid / Octree**：第一幀或 buffer 失效時重建結構並更新 skin 與位置快照，再對候選配對做 narrow-phase。
3. **碰撞回應**：彈性碰撞衝量 + 依質量反比做位置修正（避免粒子黏住）、世界邊界反彈。
4. 每幀記錄 broad／narrow／response 耗時、是否重建、候選配對數與碰撞數。

測試場景（固定 seed，可重現）：`two_particle_bounce_scenario`（除錯用）、`uniformCloud`、`explosion`、`spatialCluster`（可調聚集比例、熱點數與高速粒子比例，benchmark 預設使用）。

### Benchmark

1. 以 BruteForce 產生每幀碰撞配對作為 ground truth。
2. 執行無 buffer 的 `uniform_grid`、`octree` 基準（每步重建）。
3. 對 K ∈ {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000} 執行 `uniform_grid_skin`、`octree_skin`，每組重複 10 次取平均與標準差。

輸出欄位：總時間與各階段時間（平均／標準差）、`rebuild_count`、`avg_candidate_per_rebuild`、`correctness_ok`、`first_mismatch_frame`。

### 正確性測試

- `test_correctness.cpp`：逐幀比對 BruteForce 與 UniformGrid（開啟 skin）的碰撞配對是否完全一致。
- `test.cpp`：早期手動測試，介面已與目前版本不相容，僅供參考，不列入建置。

### 建置與執行

需求：CMake ≥ 3.14、支援 C++17 的編譯器（`glm` 已內附，無其他依賴）。

```bash
cd collision && mkdir -p build && cd build
cmake .. && cmake --build .

./bench              # 輸出 benchmark.csv
./test_correctness   # 輸出 PASS / FAIL
```

---

## system

以 `collision/` 為基礎的可視化系統，用於展示碰撞偵測模擬過程。