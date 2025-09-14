#!/usr/bin/env node
/**
 * Copyright (c) Jonathan Cardoso Machado. All Rights Reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const path = require('path')
const { Worker, isMainThread } = require('worker_threads')

const addon = require(
  path.join(__dirname, 'lib', 'binding', 'addon.node'),
)

function log(message, ...args) {
  if (isMainThread) {
    console.log(`[main thread] ${message}`, ...args)
  } else {
    console.log(`[worker thread] ${message}`, ...args)
  }
}

async function runTest() {
  return new Promise((resolve, reject) => {
    try {
      log('Testing...')

      const instance = new addon.ReproduceIssue()

      let resolveInner
      const innerPromise = new Promise((res) => {
        resolveInner = res
      })

      instance.onMessage((data) => {
        log('message callback called!', data)
        resolveInner()
      })

      innerPromise
        .then(resolve)
        .catch(reject)
    } catch (error) {
      log(`Error in runMultiTest: ${error.message}`)
      console.error(error)
      reject(error)
    }
  })
}

async function runWorkerTest() {
  return new Promise((resolve, reject) => {
    const worker = new Worker(__filename, {
      workerData: { testName: 'worker-test' },
    })

    worker.on('message', (data) => {
      if (data.type === 'log') {
        console.log(`[worker] ${data.message}`)
      }
    })

    worker.on('error', (error) => {
      reject(new Error(`Worker error: ${error.message}`))
    })

    worker.on('exit', (code) => {
      if (code === 0) {
        resolve({ success: true })
      } else {
        reject(new Error(`Worker exited with code ${code}`))
      }
    })
  })
}

async function main() {
  try {
    console.log('Testing in main thread')
    await runTest()
    console.log('✅ test in main thread completed successfully!')

    console.log('\nSpawning worker thread...')
    await runWorkerTest()
    console.log('✅ Worker thread test completed successfully!')
  } catch (error) {
    console.error('❌ Test failed:', error.message)
    process.exit(1)
  }
}

async function workerMain() {
  try {
    log('Worker started')
    await runTest()
    log('Worker completed successfully')
    process.exit(0)
  } catch (error) {
    log(`Worker error: ${error.message}`)
    process.exit(1)
  }
}

if (isMainThread) {
  main()
} else {
  workerMain()
}
