import {
  Component,
  Input,
  OnInit,
  OnDestroy,
  OnChanges,
  SimpleChanges,
  inject
} from '@angular/core';
import { CommonModule } from '@angular/common';
import { Camera } from '../../../../core/models/camera.model';
import { CameraService } from '../../../../core/services/camera.service';
import { environment } from '../../../../../environments/environment';

/**
 * Live camera view using refreshing snapshots
 * Shows the most recent snapshot from the camera buffer
 */
@Component({
  selector: 'app-live-player',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="relative w-full h-full bg-black flex items-center justify-center">
      @if (camera.connected) {
        <!-- Live Snapshot (refreshes via timestamp) -->
        <img
          [src]="getSnapshotUrl()"
          [alt]="camera.name + ' live view'"
          class="max-w-full max-h-full object-contain"
          (error)="onImageError()" />

        <!-- Live Indicator -->
        <div class="absolute top-4 right-4 px-3 py-1 bg-red-600 rounded-full text-white text-sm font-medium flex items-center gap-2">
          <span class="w-2 h-2 bg-white rounded-full animate-pulse"></span>
          LIVE
        </div>

        <!-- Camera Name -->
        <div class="absolute bottom-4 left-4 px-3 py-1 bg-black/70 rounded text-white text-sm">
          {{ camera.name }}
        </div>
      } @else {
        <!-- Camera Offline -->
        <div class="text-center text-gray-400">
          <svg class="w-16 h-16 mx-auto mb-4" fill="none" viewBox="0 0 24 24" stroke="currentColor">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M18.364 18.364A9 9 0 005.636 5.636m12.728 12.728A9 9 0 015.636 5.636m12.728 12.728L5.636 5.636" />
          </svg>
          <p class="font-medium">Camera Offline</p>
          <p class="text-sm mt-1">{{ camera.name }}</p>
        </div>
      }
    </div>
  `,
  styles: [`
    :host {
      display: block;
      width: 100%;
      height: 100%;
    }
  `]
})
export class LivePlayerComponent implements OnInit, OnDestroy, OnChanges {
  @Input() camera!: Camera;

  private cameraService = inject(CameraService);
  private refreshInterval?: number;
  private lastUpdate = Date.now();

  ngOnInit() {
    // Refresh snapshot every 2 seconds
    this.refreshInterval = window.setInterval(() => {
      this.lastUpdate = Date.now();
    }, 2000);
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes['camera'] && !changes['camera'].firstChange) {
      this.lastUpdate = Date.now();
    }
  }

  ngOnDestroy() {
    if (this.refreshInterval) {
      clearInterval(this.refreshInterval);
    }
  }

  /**
   * Get snapshot URL with cache-busting timestamp
   * This creates a "live view" by refreshing the image every 2 seconds
   */
  getSnapshotUrl(): string {
    // Use the latest snapshot from the most recent event, or a live frame endpoint
    // For now, we'll use a timestamp-based cache buster to simulate live updates
    // In production, you might want to add a /api/camera/{id}/live endpoint
    return `api/cameras/${this.camera.id}/snapshot?t=${this.lastUpdate}`;
  }

  onImageError() {
    console.warn(`Failed to load snapshot for ${this.camera.id}`);
  }
}
