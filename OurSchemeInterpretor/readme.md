# OurScheme Interpreter

以 C++ 實作的 Scheme 語言直譯器，採用經典的 **Scanner → Parser → Evaluator** 三層架構，支援完整的 S-expression 剖析、遞迴求值與豐富的錯誤偵測機制。

## 架構設計

- **Scanner**：逐字元讀取輸入，處理跳脫字元、字串邊界，將原始輸入轉換為帶有行列位置資訊（`startLine`/`startColumn`）的 Token 序列，供後續錯誤訊息精確定位。
- **Parser**：將 Token 序列遞迴建構為 S-expression 語法樹（`Node`），處理巢狀括號、引用（quote）語法糖轉換等。
- **Evaluator**：對語法樹進行遞迴求值，依 special form 分派（`FORM_DEFINE`、`FORM_IF`、`FORM_COND`、`FORM_LAMBDA`、`FORM_LET`、`FORM_AND`/`FORM_OR` 等），並維護環境（`Environment`）鏈以支援變數綁定與閉包（`Closure`）。

## 支援功能

- 基本型別：整數、浮點數、字串、符號、布林值（`T`/`nil`）
- Special forms：`define`、`if`、`cond`、`and`、`or`、`begin`、`lambda`、`let`、`set!`
- 資料操作：`cons`、`list`、`car`、`cdr`
- 算術、比較、字串處理、型別判斷（predicate）函式群
- 環境管理：`clean-environment`、`exit`
- I/O：`read`、`write`、`display-string`、`newline`
- 除錯／擴充功能：`verbose`模式切換、錯誤物件建立與判斷（`create-error-object`、`error-object?`）、`eval`

## 錯誤處理設計

針對 Scheme 直譯器常見的錯誤情境，分層設計對應例外類別（皆繼承 `runtime_error`），並在例外訊息中附上精確的行列位置：

- Scanner 層：`NoClosingQuote`（字串未閉合）
- Parser 层：`ExpectedAtomOrLeftParen`、`ExpectedRightParen`（語法結構錯誤）
- Evaluator 層：`UnboundSymbol`（未綁定變數）、`IncorrectNumberOfArguments`、`WithIncorrectArgumentType`、`AttemptToApplyNonFunction`、`DivisionByZero`，以及各 special form 專屬的格式錯誤（`DefineFormat`、`CondFormat`、`LambdaFormat`、`LetFormat`、`SetFormat`）
- 執行層級控制：`LevelOfExit`、`LevelOfDefine`、`LevelOfCleanEnvironment`（限制特定指令僅能於 top-level 執行）

## 使用技術

C++（`shared_ptr` 管理語法樹節點與環境生命週期、`unordered_map` 實作環境變數查找）