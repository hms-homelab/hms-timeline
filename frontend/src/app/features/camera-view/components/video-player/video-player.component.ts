import { Component, Input, Output, EventEmitter } from '@angular/core';
import { CommonModule } from '@angular/common';

/**
 * Recorded event video player
 * Uses native HTML5 video with custom controls
 */
@Component({
  selector: 'app-video-player',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="relative w-full h-full bg-black">
      <!-- Back to Live Button -->
      <button
        class="absolute top-4 left-4 z-10 px-4 py-2 bg-gray-800/90 hover:bg-gray-700 rounded-lg text-white text-sm font-medium transition-colors"
        (click)="onBackToLive()">
        ← Back to Live
      </button>

      <!-- Video Player -->
      <video
        class="w-full h-full"
        [src]="recordingUrl"
        controls
        autoplay>
        Your browser does not support the video tag.
      </video>
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
export class VideoPlayerComponent {
  @Input() recordingUrl!: string;
  @Output() backToLive = new EventEmitter<void>();

  onBackToLive() {
    this.backToLive.emit();
  }
}
