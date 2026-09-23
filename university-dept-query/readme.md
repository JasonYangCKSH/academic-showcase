# University Department Search: 2-3 Tree + AVL Tree

以兩種平衡樹儲存大學校系統計資料：**2-3 Tree** 以校名為 key，**AVL Tree** 以科系名稱為 key，並支援跨樹交集查詢，例如找出「某校 × 某科系」的所有紀錄。

## 資料結構設計

| 結構 | Key | 平衡機制 |
|---|---|---|
| 2-3 Tree | `schoolName` | 節點容納 1～2 個 key（2-node／3-node）；葉節點溢位時以中間值向上分裂（`splitNode`），必要時遞迴分裂父節點 |
| AVL Tree | `department` | 插入後檢查左右子樹高度差，超過 1 即依 LL／LR／RL／RR 執行單旋轉或雙旋轉（`rotateLeft`／`rotateRight`） |

兩棵樹的每個 key（`SchoolKey`）皆以 `vector<Data*>` 保存所有相同 key 值的紀錄，因此同一校名或科系可對應多筆資料。

## 演算法流程

<center>

![2-3 Tree 建立流程圖](image.png)

**圖 1**　2-3 Tree 建立流程圖

</center>

## 查詢功能

1. 輸入校名與科系名稱（可用 `*` 表示不限），分別在兩棵樹中取出候選集合。
2. 對兩組候選取交集，依原始輸入順序（`inputOrder`）排序後輸出。
3. 若任一條件為 `*`，則以另一棵樹的查詢結果直接與完整資料集比對。

<center>

![查詢資訊工程學系的輸出結果](image-1.png)

**圖 2**　輸入「資訊工程學系」後，系統自檔案中篩選並輸出符合的資料

</center>

## 使用技術

C++：樹狀結構以指標實作，並以 `vector` 儲存節點內的多筆紀錄及進行交集運算。