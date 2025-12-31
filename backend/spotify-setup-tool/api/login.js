const querystring = require('querystring');

module.exports = (req, res) => {
  const { device_id, client_id, client_secret } = req.query;

  if (!device_id || !client_id || !client_secret) {
    return res.status(400).send('Device ID, Client ID, and Client Secret are all required.');
  }

  const statePayload = {
    device_id: device_id,
    client_id: client_id,
    client_secret: client_secret,
  };
  const state = Buffer.from(JSON.stringify(statePayload)).toString('base64');

  const scope = 'user-read-currently-playing user-read-playback-state';
  const redirect_uri = `https://${req.headers.host}/api/callback`;

  const query = querystring.stringify({
    response_type: 'code',
    client_id: client_id,
    scope: scope,
    redirect_uri: redirect_uri,
    state: state,
  });

  res.redirect(`https://accounts.spotify.com/authorize?${query}`);
};
