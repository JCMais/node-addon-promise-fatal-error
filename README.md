# node-addon-promise-fatal-error

Build
```bash
pnpm gyp rebuild --debug
```

Run the test calling MakeCallback (will crash)
```bash
pnpm run reproduce-issue
```

Run the test calling ThreadSafeFunction (will not crash)
```bash
pnpm run reproduce-issue -- USE_THREAD_SAFE=true
```
