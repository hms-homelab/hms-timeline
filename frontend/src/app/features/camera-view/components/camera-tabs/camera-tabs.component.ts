import { Component, Input, Output, EventEmitter } from '@angular/core';
import { CommonModule } from '@angular/common';
import { Camera } from '../../../../core/models/camera.model';

/**
 * Camera selector tabs component
 * Displays horizontal tabs for switching between cameras
 */
@Component({
  selector: 'app-camera-tabs',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="flex overflow-x-auto">
      @for (camera of cameras; track camera.id) {
        <button
          type="button"
          class="flex-1 min-w-[120px] px-6 py-4 text-center border-b-2 transition-all"
          [class.border-blue-500]="selectedCamera?.id === camera.id"
          [class.text-blue-500]="selectedCamera?.id === camera.id"
          [class.border-transparent]="selectedCamera?.id !== camera.id"
          [class.text-gray-400]="selectedCamera?.id !== camera.id"
          [class.hover:text-white]="selectedCamera?.id !== camera.id"
          (click)="onCameraClick(camera)">
          <div class="font-medium">{{ camera.name }}</div>
          @if (!camera.connected) {
            <div class="text-xs text-red-500 mt-1">Offline</div>
          }
        </button>
      }
    </div>
  `
})
export class CameraTabsComponent {
  @Input() cameras: Camera[] = [];
  @Input() selectedCamera: Camera | null = null;
  @Output() cameraChange = new EventEmitter<Camera>();

  onCameraClick(camera: Camera) {
    this.cameraChange.emit(camera);
  }
}
