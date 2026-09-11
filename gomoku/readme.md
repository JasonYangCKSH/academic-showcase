# Gomoku White-Side AI（五子棋白方 AI）
- **engine_minimax.py**為黑方對手
- **hw1_11220105.py**為白方對手

以 Minimax + Alpha-Beta Pruning 為核心，搭配自訂啟發式評分系統實作的五子棋白方對弈程式，需於 5 秒時限內透過 stdin/stdout 與裁判程式通訊完成對弈。

**核心設計**：`move_priority()` 負責候選步排序以提升剪枝效率，`evaluate_board()` 負責葉節點全局評估；評分依連棋型態（五連、活四、活三...）、複合威脅（雙活四、雙活三、四三）、Position Heuristic 與 Black Forbidden Trap 逐層累加，並針對白黑雙方採不對稱權重設計。

**與 AI 協作**：由生成式 AI 協助搭建 Minimax 搜尋框架，細節評分邏輯（複合威脅判斷、禁手陷阱、不對稱權重）為實測後自行設計並交由 AI 擴充實作。

**已知限制**：Time limit 判斷較為粗暴，搜尋深度 ≥ 6 時偶有防守失準情況，未來可改採迭代加深（iterative deepening）改善。

**使用技術**：Python