
const req = require('./servercomm.dev.js');

const crypto = require('crypto');

// input values
let request = {
	name: 'List-Backups',
	rosiVersion: 'ROSI-Plugin Vers. 0.1',
	userId: 'maxi',
	data: ''
};

// Calculate checksum
let hash = crypto.createHash('sha256');
hash.update(request.name + request.rosiVersion + request.userId + request.data);
let checksum = hash.digest('hex');

console.log('hash generated: ' + checksum);

req.topayserver('http://localhost:12000/', {'User-Agent': request.rosiVersion, 
											'Rosi-Request' : request.name,
											'User-Id' : request.userId,
											'Request-Checksum' : checksum }, request.data, function(data){
	
		console.log('Data Received back: ', data);
});
