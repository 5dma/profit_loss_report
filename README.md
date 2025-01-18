# Profit Loss Report
Report designer for customized profit-loss reports extracted from GnuCash database. For much more detail, run `doxygen Doxyfile`.

Requires gcc, gtk-3.0, sqlite3

## Compile command
```
Make
```

# Sample JSON configuration file
```
{
  "start_date" : "2024-01-16 00:00:00",
  "end_date" : null,
  "sqlite_file" : "/path/filename.gnucash.sqlite.gnucash",
  "output_file" : "/path/profit_loss.html",
  "properties" : [
    {
      "code" : "12201",
      "guid" : "e5f8ec46fbd34f89b1547f53382d7304",
      "income_accounts" : [
        "fb38e7e53effab63c885bcaa6f6f8cec"
      ],
      "expense_accounts" : [
        "91ae5d1c7385ac5c9cb8d2dd45c31094",
        "544bf2f9f0d761811a9f4f40fb7187be",
        "5d14b1a8850b287b9831d5db9632b928",
        "802f80427daa0753de713c3f89f2b06b",
        "c4d52bd95fd9ebfae5e55921f3364934",
        "25bb8acb2f57a29a52de55703d3186b0",
        "3cf74ed7a569efe76806adb1ea9e4217"
      ]
    }
  ]
}
```
