"""Offline integration test of the real command loop, using stub engine services."""
import json
import subprocess
import sys

commands = [
    'not json',
    json.dumps({'action': 'play'}),
    json.dumps({'action': 'login', 'username': 'tester', 'password': 'wrong'}),
    json.dumps({'action': 'login', 'username': 'tester', 'password': 'not-a-real-password'}),
    json.dumps({'action': 'logout'}),
    json.dumps({'action': 'guest'}),
    json.dumps({'action': 'play'}),
    json.dumps({'action': 'logout'}),
]
result = subprocess.run([sys.argv[1], '--gui-backend'], input='\n'.join(commands) + '\n',
                        text=True, capture_output=True, timeout=10)
assert result.returncode == 0, result.stderr
prefix = '@@BEAMMP_GUI@@'
events = [json.loads(line[len(prefix):]) for line in result.stderr.splitlines() if line.startswith(prefix)]
assert [event['event'] for event in events] == [
    'ready', 'error', 'error', 'auth', 'auth', 'auth', 'auth', 'preparing', 'playing', 'error'], events
assert events[3]['success'] is False
assert events[4]['authenticated'] is True and events[4]['username'] == 'tester'
assert events[5]['authenticated'] is False
assert events[6]['authenticated'] is True and events[6]['username'] == 'guest123'
for secret in ['not-a-real-password', 'TEST_PRIVATE_KEY', 'TEST_PUBLIC_KEY']:
    assert secret not in result.stdout + result.stderr, 'Secret exposed in output'
print('PASS: malformed input, auth requirement, failed/successful login, logout, guest, play, account lock and key/password redaction.')
