## 1.0.1
### Added
- Caché interna de parámetros Gen y helpers de sincronización APVTS ↔ Gen (`rebuildGenParameterLookup`, `enqueueAllParametersForAudioThread`, `pushGenParamByName`).

### Changed
- Restauración de estado ahora reencola parámetros con `enqueueAllParametersForAudioThread()` en lugar del bucle manual.
- La carga de presets sincroniza con Gen usando `pushGenParamByName` (sin escribir directamente en el estado Gen).
- Mantención de clamps críticos en ataque/reléase (`d_ATK`, `e_REL`) con actualización del APVTS cuando valores son demasiado bajos.

### Removed
- Código heredado de versiones anteriores que ya no correspondía al flujo de sincronización actual (bucle manual de reenvío a Gen).

## 1.0.0
- Versión inicial del plugin.