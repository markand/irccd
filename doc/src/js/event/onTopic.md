# Event onTopic

This event is triggered when someone changed the channel's topic.

# Synopsis

```javascript
function onTopic(server, origin, channel, topic)
```

# Arguments

- server: the current server,
- origin: the person who changed the topic,
- channel: the channel,
- topic: the new topic (may be empty).
