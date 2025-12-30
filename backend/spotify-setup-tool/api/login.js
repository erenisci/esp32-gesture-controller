const querystring = require('querystring');

module.exports = (req, res) => {
  const device_id = req.query.device_id;

  if (!device_id) {
    return res.status(400).send('Device ID (MAC Address) is required.');
  }

  const state = device_id;
  const scope = 'user-read-currently-playing user-read-playback-state';
  const redirect_uri = `https://${req.headers.host}/api/callback`;

  const query = querystring.stringify({
    response_type: 'code',
    client_id: process.env.SPOTIFY_CLIENT_ID,
    scope: scope,
    redirect_uri: redirect_uri,
    state: state,
  });

  res.redirect(`https://accounts.spotify.com/authorize?${query}`);
};
