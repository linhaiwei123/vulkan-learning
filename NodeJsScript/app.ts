const qbEngine = require('../QbEngine/x64/Debug/NodeJsBinding/NodeJsBinding');
console.log('launch qb-engine');
const ret = qbEngine.run();
console.log(ret);