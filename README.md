# node-addon-promise-fatal-error

Build
```bash
pnpm gyp rebuild --debug
```

Run the test calling with Call (will not crash)
```bash
node reproduce-issue.js
```

Run the test calling with MakeCallback (will crash)
```bash
USE_MAKE_CALLBACK=true node reproduce-issue.js
```
