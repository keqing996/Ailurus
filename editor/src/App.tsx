import { useEffect, useMemo, useRef, useState } from 'react';

type ConnectionState = 'disconnected' | 'connecting' | 'full-sync' | 'live-delta';

type Vec3 = [number, number, number];
type Vec4 = [number, number, number, number];

type EntityState = {
  guid: number;
  name: string;
  parent: number | null;
  transform: {
    position: Vec3;
    rotationEuler: Vec3;
    scale: Vec3;
  };
};

type RenderSettings = {
  clearColor: Vec4;
  ambientColor: Vec3;
  ambientStrength: number;
  shadow: {
    constantBias: number;
    slopeScale: number;
    normalOffset: number;
  };
  vsyncEnabled: boolean;
  msaaEnabled: boolean;
  skyboxEnabled: boolean;
  toneMapping: {
    exposure: number;
    gamma: number;
  };
  bloom: {
    threshold: number;
    softKnee: number;
    intensity: number;
    blendFactor: number;
  };
};

type ServerEnvelope = {
  kind: 'fullSnapshot' | 'deltaBatch' | 'error';
  sessionId: number;
  revision: number;
  payload: any;
};

type EntityDraft = {
  name: string;
  position: [string, string, string];
  rotationEuler: [string, string, string];
  scale: [string, string, string];
};

type RenderDraft = {
  clearColor: [string, string, string, string];
  ambientColor: [string, string, string];
  ambientStrength: string;
  shadow: {
    constantBias: string;
    slopeScale: string;
    normalOffset: string;
  };
  vsyncEnabled: boolean;
  msaaEnabled: boolean;
  skyboxEnabled: boolean;
  toneMapping: {
    exposure: string;
    gamma: string;
  };
  bloom: {
    threshold: string;
    softKnee: string;
    intensity: string;
    blendFactor: string;
  };
};

const websocketUrl = 'ws://127.0.0.1:12138';

const toVec3Draft = (value: Vec3): [string, string, string] => value.map((entry) => entry.toString()) as [string, string, string];
const toVec4Draft = (value: Vec4): [string, string, string, string] => value.map((entry) => entry.toString()) as [string, string, string, string];

const createEntityDraft = (entity: EntityState | undefined): EntityDraft => ({
  name: entity?.name ?? '',
  position: toVec3Draft(entity?.transform.position ?? [0, 0, 0]),
  rotationEuler: toVec3Draft(entity?.transform.rotationEuler ?? [0, 0, 0]),
  scale: toVec3Draft(entity?.transform.scale ?? [1, 1, 1]),
});

const createRenderDraft = (settings: RenderSettings | null): RenderDraft => ({
  clearColor: toVec4Draft(settings?.clearColor ?? [0, 0, 0, 1]),
  ambientColor: toVec3Draft(settings?.ambientColor ?? [1, 1, 1]),
  ambientStrength: (settings?.ambientStrength ?? 0).toString(),
  shadow: {
    constantBias: (settings?.shadow.constantBias ?? 0).toString(),
    slopeScale: (settings?.shadow.slopeScale ?? 0).toString(),
    normalOffset: (settings?.shadow.normalOffset ?? 0).toString(),
  },
  vsyncEnabled: settings?.vsyncEnabled ?? true,
  msaaEnabled: settings?.msaaEnabled ?? true,
  skyboxEnabled: settings?.skyboxEnabled ?? true,
  toneMapping: {
    exposure: (settings?.toneMapping.exposure ?? 1).toString(),
    gamma: (settings?.toneMapping.gamma ?? 2.2).toString(),
  },
  bloom: {
    threshold: (settings?.bloom.threshold ?? 1).toString(),
    softKnee: (settings?.bloom.softKnee ?? 0.5).toString(),
    intensity: (settings?.bloom.intensity ?? 0.5).toString(),
    blendFactor: (settings?.bloom.blendFactor ?? 0.7).toString(),
  },
});

function App() {
  const [connectionState, setConnectionState] = useState<ConnectionState>('connecting');
  const [entities, setEntities] = useState<EntityState[]>([]);
  const [renderSettings, setRenderSettings] = useState<RenderSettings | null>(null);
  const [selectedGuid, setSelectedGuid] = useState<number | null>(null);
  const [entityDraft, setEntityDraft] = useState<EntityDraft>(createEntityDraft(undefined));
  const [renderDraft, setRenderDraft] = useState<RenderDraft>(createRenderDraft(null));
  const [statusText, setStatusText] = useState('Connecting to renderer...');
  const [errorText, setErrorText] = useState<string | null>(null);

  const socketRef = useRef<WebSocket | null>(null);
  const reconnectTimerRef = useRef<number | null>(null);
  const sessionIdRef = useRef<number | null>(null);
  const revisionRef = useRef(0);
  const entitiesRef = useRef<EntityState[]>([]);
  const renderSettingsRef = useRef<RenderSettings | null>(null);

  const selectedEntity = entities.find((entity) => entity.guid === selectedGuid);

  useEffect(() => {
    entitiesRef.current = entities;
    if (selectedGuid !== null && !entities.some((entity) => entity.guid === selectedGuid)) {
      setSelectedGuid(entities[0]?.guid ?? null);
    }
  }, [entities, selectedGuid]);

  useEffect(() => {
    renderSettingsRef.current = renderSettings;
  }, [renderSettings]);

  useEffect(() => {
    setEntityDraft(createEntityDraft(selectedEntity));
  }, [selectedEntity]);

  useEffect(() => {
    setRenderDraft(createRenderDraft(renderSettings));
  }, [renderSettings]);

  useEffect(() => {
    const connect = () => {
      setConnectionState('connecting');
      setStatusText('Connecting to renderer...');
      const socket = new WebSocket(websocketUrl);
      socketRef.current = socket;

      socket.onopen = () => {
        setConnectionState('full-sync');
        setStatusText('Socket open. Waiting for full snapshot...');
        setErrorText(null);
      };

      socket.onmessage = (event) => {
        const envelope = JSON.parse(event.data) as ServerEnvelope;
        handleEnvelope(envelope);
      };

      socket.onerror = () => {
        setErrorText('Socket error. Waiting to reconnect...');
      };

      socket.onclose = () => {
        socketRef.current = null;
        sessionIdRef.current = null;
        revisionRef.current = 0;
        setConnectionState('disconnected');
        setStatusText('Connection closed. Reconnecting...');
        reconnectTimerRef.current = window.setTimeout(connect, 1000);
      };
    };

    connect();

    return () => {
      if (reconnectTimerRef.current !== null) {
        window.clearTimeout(reconnectTimerRef.current);
      }
      socketRef.current?.close();
    };
  }, []);

  const tree = useMemo(() => {
    const grouped = new Map<number | null, EntityState[]>();
    for (const entity of entities) {
      const siblings = grouped.get(entity.parent) ?? [];
      siblings.push(entity);
      grouped.set(entity.parent, siblings);
    }

    for (const siblings of grouped.values()) {
      siblings.sort((left, right) => left.guid - right.guid);
    }

    return grouped;
  }, [entities]);

  const handleEnvelope = (envelope: ServerEnvelope) => {
    if (envelope.kind === 'fullSnapshot') {
      const nextEntities = envelope.payload.entities as EntityState[];
      const nextRenderSettings = envelope.payload.renderSettings as RenderSettings;
      sessionIdRef.current = envelope.sessionId;
      revisionRef.current = envelope.revision;
      setEntities(nextEntities);
      setRenderSettings(nextRenderSettings);
      setSelectedGuid((currentGuid) => currentGuid ?? nextEntities[0]?.guid ?? null);
      setConnectionState('live-delta');
      setStatusText(`Live with revision ${envelope.revision}.`);
      setErrorText(null);
      return;
    }

    if (envelope.kind === 'deltaBatch') {
      if (envelope.revision !== revisionRef.current + 1) {
        setConnectionState('full-sync');
        setStatusText('Revision gap detected. Requesting full sync...');
        requestFullSync();
        return;
      }

      const nextEntities = applyDeltaEntities(entitiesRef.current, envelope.payload.changes);
      const nextRenderSettings = applyDeltaRenderSettings(renderSettingsRef.current, envelope.payload.changes);
      revisionRef.current = envelope.revision;
      setEntities(nextEntities);
      setRenderSettings(nextRenderSettings);
      setConnectionState('live-delta');
      setStatusText(`Live with revision ${envelope.revision}.`);
      return;
    }

    if (envelope.kind === 'error') {
      setErrorText(envelope.payload.message as string);
      if (envelope.payload.fullResyncScheduled) {
        setConnectionState('full-sync');
        setStatusText('Renderer scheduled a full resync...');
      }
    }
  };

  const sendCommand = (payload: Record<string, unknown>) => {
    if (socketRef.current === null || socketRef.current.readyState !== WebSocket.OPEN || sessionIdRef.current === null) {
      setErrorText('Renderer is not connected yet.');
      return;
    }

    socketRef.current.send(JSON.stringify({
      kind: 'command',
      sessionId: sessionIdRef.current,
      baseRevision: revisionRef.current,
      payload,
    }));
  };

  const requestFullSync = () => {
    sendCommand({ type: 'requestFullSync' });
  };

  const applyEntityDraft = () => {
    if (selectedGuid === null) {
      return;
    }

    sendCommand({
      type: 'setEntityName',
      guid: selectedGuid,
      name: entityDraft.name,
    });
    sendCommand({
      type: 'setEntityTransform',
      guid: selectedGuid,
      position: entityDraft.position.map(Number),
      rotationEuler: entityDraft.rotationEuler.map(Number),
      scale: entityDraft.scale.map(Number),
    });
  };

  const applyRenderDraft = () => {
    sendCommand({
      type: 'setRenderSettings',
      clearColor: renderDraft.clearColor.map(Number),
      ambientColor: renderDraft.ambientColor.map(Number),
      ambientStrength: Number(renderDraft.ambientStrength),
      shadow: {
        constantBias: Number(renderDraft.shadow.constantBias),
        slopeScale: Number(renderDraft.shadow.slopeScale),
        normalOffset: Number(renderDraft.shadow.normalOffset),
      },
      vsyncEnabled: renderDraft.vsyncEnabled,
      msaaEnabled: renderDraft.msaaEnabled,
      skyboxEnabled: renderDraft.skyboxEnabled,
      toneMapping: {
        exposure: Number(renderDraft.toneMapping.exposure),
        gamma: Number(renderDraft.toneMapping.gamma),
      },
      bloom: {
        threshold: Number(renderDraft.bloom.threshold),
        softKnee: Number(renderDraft.bloom.softKnee),
        intensity: Number(renderDraft.bloom.intensity),
        blendFactor: Number(renderDraft.bloom.blendFactor),
      },
    });
  };

  const renderEntityBranch = (parent: number | null, depth = 0): JSX.Element[] => {
    const children = tree.get(parent) ?? [];
    return children.flatMap((entity) => [
      <button
        key={entity.guid}
        className={`entity-row ${selectedGuid === entity.guid ? 'selected' : ''}`}
        style={{ paddingLeft: `${depth * 16 + 12}px` }}
        onClick={() => setSelectedGuid(entity.guid)}
        type="button"
      >
        <span className="entity-name">{entity.name}</span>
        <span className="entity-guid">#{entity.guid}</span>
      </button>,
      ...renderEntityBranch(entity.guid, depth + 1),
    ]);
  };

  return (
    <div className="shell">
      <header className="hero">
        <div>
          <p className="eyebrow">Ailurus External Editor</p>
          <h1>Browser bridge for live scene and render tuning</h1>
        </div>
        <div className={`connection-chip state-${connectionState}`}>
          <span>{connectionState}</span>
          <strong>{statusText}</strong>
        </div>
      </header>

      {errorText !== null && <div className="error-banner">{errorText}</div>}

      <main className="layout">
        <section className="panel tree-panel">
          <div className="panel-header">
            <h2>Scene</h2>
            <button type="button" onClick={requestFullSync}>Resync</button>
          </div>
          <div className="entity-list">{renderEntityBranch(null)}</div>
        </section>

        <section className="panel inspector-panel">
          <div className="panel-header">
            <h2>Entity Inspector</h2>
            <button type="button" onClick={applyEntityDraft} disabled={selectedGuid === null}>Apply</button>
          </div>
          {selectedEntity === undefined ? (
            <p className="empty-state">Select an entity from the scene tree.</p>
          ) : (
            <div className="form-grid">
              <label>
                <span>Name</span>
                <input value={entityDraft.name} onChange={(event) => setEntityDraft({ ...entityDraft, name: event.target.value })} />
              </label>
              <VectorField label="Position" values={entityDraft.position} onChange={(position) => setEntityDraft({ ...entityDraft, position })} />
              <VectorField label="Rotation (rad)" values={entityDraft.rotationEuler} onChange={(rotationEuler) => setEntityDraft({ ...entityDraft, rotationEuler })} />
              <VectorField label="Scale" values={entityDraft.scale} onChange={(scale) => setEntityDraft({ ...entityDraft, scale })} />
            </div>
          )}
        </section>

        <section className="panel render-panel">
          <div className="panel-header">
            <h2>Render Settings</h2>
            <button type="button" onClick={applyRenderDraft} disabled={renderSettings === null}>Apply</button>
          </div>
          {renderSettings === null ? (
            <p className="empty-state">Waiting for the renderer snapshot...</p>
          ) : (
            <div className="form-grid">
              <VectorField4 label="Clear Color" values={renderDraft.clearColor} onChange={(clearColor) => setRenderDraft({ ...renderDraft, clearColor })} />
              <VectorField label="Ambient Color" values={renderDraft.ambientColor} onChange={(ambientColor) => setRenderDraft({ ...renderDraft, ambientColor })} />
              <ScalarField label="Ambient Strength" value={renderDraft.ambientStrength} onChange={(ambientStrength) => setRenderDraft({ ...renderDraft, ambientStrength })} />
              <ScalarField label="Shadow Constant Bias" value={renderDraft.shadow.constantBias} onChange={(constantBias) => setRenderDraft({ ...renderDraft, shadow: { ...renderDraft.shadow, constantBias } })} />
              <ScalarField label="Shadow Slope Scale" value={renderDraft.shadow.slopeScale} onChange={(slopeScale) => setRenderDraft({ ...renderDraft, shadow: { ...renderDraft.shadow, slopeScale } })} />
              <ScalarField label="Shadow Normal Offset" value={renderDraft.shadow.normalOffset} onChange={(normalOffset) => setRenderDraft({ ...renderDraft, shadow: { ...renderDraft.shadow, normalOffset } })} />
              <ToggleField label="VSync" checked={renderDraft.vsyncEnabled} onChange={(vsyncEnabled) => setRenderDraft({ ...renderDraft, vsyncEnabled })} />
              <ToggleField label="MSAA" checked={renderDraft.msaaEnabled} onChange={(msaaEnabled) => setRenderDraft({ ...renderDraft, msaaEnabled })} />
              <ToggleField label="Skybox" checked={renderDraft.skyboxEnabled} onChange={(skyboxEnabled) => setRenderDraft({ ...renderDraft, skyboxEnabled })} />
              <ScalarField label="Tone Mapping Exposure" value={renderDraft.toneMapping.exposure} onChange={(exposure) => setRenderDraft({ ...renderDraft, toneMapping: { ...renderDraft.toneMapping, exposure } })} />
              <ScalarField label="Tone Mapping Gamma" value={renderDraft.toneMapping.gamma} onChange={(gamma) => setRenderDraft({ ...renderDraft, toneMapping: { ...renderDraft.toneMapping, gamma } })} />
              <ScalarField label="Bloom Threshold" value={renderDraft.bloom.threshold} onChange={(threshold) => setRenderDraft({ ...renderDraft, bloom: { ...renderDraft.bloom, threshold } })} />
              <ScalarField label="Bloom Soft Knee" value={renderDraft.bloom.softKnee} onChange={(softKnee) => setRenderDraft({ ...renderDraft, bloom: { ...renderDraft.bloom, softKnee } })} />
              <ScalarField label="Bloom Intensity" value={renderDraft.bloom.intensity} onChange={(intensity) => setRenderDraft({ ...renderDraft, bloom: { ...renderDraft.bloom, intensity } })} />
              <ScalarField label="Bloom Blend Factor" value={renderDraft.bloom.blendFactor} onChange={(blendFactor) => setRenderDraft({ ...renderDraft, bloom: { ...renderDraft.bloom, blendFactor } })} />
            </div>
          )}
        </section>
      </main>
    </div>
  );
}

function VectorField(props: { label: string; values: [string, string, string]; onChange: (next: [string, string, string]) => void }) {
  return (
    <label>
      <span>{props.label}</span>
      <div className="triple-input">
        {props.values.map((value, index) => (
          <input
            key={`${props.label}-${index}`}
            value={value}
            onChange={(event) => {
              const next = [...props.values] as [string, string, string];
              next[index] = event.target.value;
              props.onChange(next);
            }}
          />
        ))}
      </div>
    </label>
  );
}

function VectorField4(props: { label: string; values: [string, string, string, string]; onChange: (next: [string, string, string, string]) => void }) {
  return (
    <label>
      <span>{props.label}</span>
      <div className="quad-input">
        {props.values.map((value, index) => (
          <input
            key={`${props.label}-${index}`}
            value={value}
            onChange={(event) => {
              const next = [...props.values] as [string, string, string, string];
              next[index] = event.target.value;
              props.onChange(next);
            }}
          />
        ))}
      </div>
    </label>
  );
}

function ScalarField(props: { label: string; value: string; onChange: (next: string) => void }) {
  return (
    <label>
      <span>{props.label}</span>
      <input value={props.value} onChange={(event) => props.onChange(event.target.value)} />
    </label>
  );
}

function ToggleField(props: { label: string; checked: boolean; onChange: (next: boolean) => void }) {
  return (
    <label className="toggle-field">
      <span>{props.label}</span>
      <input type="checkbox" checked={props.checked} onChange={(event) => props.onChange(event.target.checked)} />
    </label>
  );
}

function applyDeltaEntities(current: EntityState[], changes: any[]): EntityState[] {
  const next = new Map<number, EntityState>(current.map((entity) => [entity.guid, entity]));

  for (const change of changes) {
    if (change.type === 'entityCreated') {
      next.set(change.entity.guid, change.entity as EntityState);
      continue;
    }

    if (change.type === 'entityDestroyed') {
      next.delete(change.guid as number);
      continue;
    }

    if (change.type === 'entityPatched') {
      const entity = next.get(change.guid as number);
      if (entity === undefined) {
        continue;
      }

      next.set(change.guid as number, {
        ...entity,
        name: change.name ?? entity.name,
        parent: change.parent === undefined ? entity.parent : (change.parent as number | null),
        transform: change.transform === undefined ? entity.transform : (change.transform as EntityState['transform']),
      });
    }
  }

  return Array.from(next.values()).sort((left, right) => left.guid - right.guid);
}

function applyDeltaRenderSettings(current: RenderSettings | null, changes: any[]): RenderSettings | null {
  let next = current;
  for (const change of changes) {
    if (change.type === 'renderSettingsUpdated') {
      next = change.renderSettings as RenderSettings;
    }
  }
  return next;
}

export default App;