# Academic Showcase

中原大學資訊工程學系 楊耀甯（Jason Yang）大學期間的作品整理，供推甄備審參考。各專案資料夾內附有獨立的README，說明設計細節與執行結果。

## 專案一覽

| 專案 | 簡介 | 技術 |
|---|---|---|
| [N-body 粒子碰撞偵測](nbody_simulation/) | **大學專題**。比較Uniform Grid／Octree結合Verlet List自適應skin的碰撞偵測效能，並附3D互動視覺化 | C++、TypeScript、React Three Fiber |
| [OurScheme 直譯器](OurSchemeInterpretor/) | Scanner → Parser → Evaluator 三層架構的Scheme直譯器，支援閉包與完整錯誤偵測 | C++ |
| [五子棋白方 AI](gomoku/) | Minimax + Alpha-Beta剪枝，搭配複合威脅與禁手陷阱的啟發式評分 | Python |
| [平行排序效能比較](thread_computing/) | 比較單流程、multi-process、multi-thread的排序效能，分析理論複雜度與實際開銷的落差 | C++、fork／mmap、std::thread |
| [Julia Set 動畫](drawJuliaSet/) | 以手寫ARM組合語言、定點整數運算繪製Julia Set，直接寫入Frame Buffer| ARM Assembly、C |
| [大學校系查詢系統](university-dept-query/) | 以2-3 Tree與AVL Tree分別索引校名與科系，支援交集查詢 | C++ |
| [雙廚師訂單排程模擬](kitchen-simulator/) | 以佇列模擬點餐系統，比較FIFO與SQF排程的延遲與失敗率 | C++ |

## 涵蓋領域

物理模擬與空間資料結構 · 程式語言實作 · 五子棋AI · 作業系統（process／thread） · 組合語言 · 資料結構與演算法
