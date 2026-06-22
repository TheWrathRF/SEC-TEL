-- valid, decrypted telemetry coming from the UAV
CREATE TABLE IF NOT EXISTS clean_telemetry (
    id          BIGSERIAL PRIMARY KEY,
    ts          BIGINT NOT NULL,            -- UAV timestamp (unix seconds)
    latitude    DOUBLE PRECISION NOT NULL,
    longitude   DOUBLE PRECISION NOT NULL,
    altitude    REAL NOT NULL,
    speed       REAL NOT NULL,
    battery     REAL NOT NULL,
    received_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- packets that failed validation (corrupted / jammed)
CREATE TABLE IF NOT EXISTS attack_logs (
    id          BIGSERIAL PRIMARY KEY,
    received_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    source_addr TEXT,
    source_port INTEGER,
    packet_size INTEGER,
    reason      TEXT NOT NULL
);
