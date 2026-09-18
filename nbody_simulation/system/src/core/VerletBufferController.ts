import { distance, type ParticleData, type RebuildEvent, type SpatialStructure } from './types';

export class VerletBufferController {
  public RC: number;
  public K: number;
  private step = 0;
  private lastTrigger = 0;

  constructor(RC: number, K: number, private readonly dt: number) { this.RC = RC; this.K = K; }
  computeSkin(particle: ParticleData): number { return this.K * Math.hypot(particle.velocity.x, particle.velocity.y, particle.velocity.z) * this.dt; }
  updateSkins(particles: ParticleData[], caps?: number[]): void {
    particles.forEach((particle, index) => {
      particle.skin = this.computeSkin(particle);
      if (caps) particle.skin = Math.min(particle.skin, caps[index]);
    });
  }
  isListValid(particles: ParticleData[]): boolean { return particles.every((particle) => distance(particle.position, particle.positionAtLastBroadPhase) <= particle.skin); }
  rebuild(structure: SpatialStructure, particles: ParticleData[], step: number): RebuildEvent {
    this.step = step;
    const trigger = particles.reduce((max, particle) => {
      const displacement = distance(particle.position, particle.positionAtLastBroadPhase);
      return displacement > max.displacement ? { particle, displacement } : max;
    }, { particle: particles[0], displacement: 0 });
    structure.build(particles);
    this.updateSkins(particles, structure.getSkinCaps?.(particles));
    particles.forEach((particle) => { particle.positionAtLastBroadPhase = { ...particle.position }; });
    this.lastTrigger = trigger.particle?.id ?? 0;
    return { step, triggeredByParticleId: this.lastTrigger, displacement: trigger.displacement, skinAtTrigger: trigger.particle?.skin ?? 0 };
  }
}
