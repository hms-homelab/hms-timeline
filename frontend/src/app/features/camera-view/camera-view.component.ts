import { Component, OnInit, OnDestroy, signal, inject, effect } from '@angular/core';
import { CommonModule } from '@angular/common';
import { toSignal } from '@angular/core/rxjs-interop';
import { ActivatedRoute, Router } from '@angular/router';
import { Title } from '@angular/platform-browser';
import { combineLatest } from 'rxjs';

import { CameraService } from '../../core/services/camera.service';
import { EventsService } from '../../core/services/events.service';
import { Camera } from '../../core/models/camera.model';
import { DetectionEvent, SearchResult, PeriodicSnapshot } from '../../core/models/event.model';
import { toRelativeMediaUrl } from '../../core/utils/media-url';

import { LoadingSpinnerComponent } from '../../shared/components/loading-spinner/loading-spinner.component';
import { ErrorMessageComponent } from '../../shared/components/error-message/error-message.component';
import { CameraTabsComponent } from './components/camera-tabs/camera-tabs.component';
import { DatePickerComponent } from './components/date-picker/date-picker.component';
import { TimelineComponent } from './components/timeline/timeline.component';
import { EventListComponent } from './components/event-list/event-list.component';
import { SearchBarComponent, SearchQuery } from './components/search-bar/search-bar.component';

/**
 * Main container component for camera view
 * Manages state using Angular Signals and passes data down to child components
 */
@Component({
  selector: 'app-camera-view',
  standalone: true,
  imports: [
    CommonModule,
    LoadingSpinnerComponent,
    ErrorMessageComponent,
    CameraTabsComponent,
    DatePickerComponent,
    TimelineComponent,
    EventListComponent,
    SearchBarComponent
  ],
  templateUrl: './camera-view.component.html',
  styleUrls: ['./camera-view.component.scss']
})
export class CameraViewComponent implements OnInit, OnDestroy {
  private cameraService = inject(CameraService);
  private eventsService = inject(EventsService);
  private route = inject(ActivatedRoute);
  private router = inject(Router);
  private titleService = inject(Title);

  // Writable Signals (User Input)
  selectedCamera = signal<Camera | null>(null);
  selectedDate = signal<Date>(new Date());
  viewMode = signal<'live' | 'recording' | 'snapshot'>('live');
  selectedRecordingUrl = signal<string | null>(null);
  selectedEventId = signal<string | null>(null);
  videoError = signal<boolean>(false);

  // Load cameras on init
  cameras$ = this.cameraService.getCameraStatus();
  cameras = toSignal(this.cameras$, { initialValue: [] });

  // Events signal (will be loaded when camera/date changes)
  events = signal<DetectionEvent[]>([]);
  isLoadingEvents = signal<boolean>(false);

  // Search mode
  isSearchMode = signal(false);
  searchResults = signal<SearchResult[]>([]);
  isSearchLoading = signal(false);
  searchInfo = signal<{ mode: string; count: number } | null>(null);
  periodicSnapshots = signal<PeriodicSnapshot[]>([]);

  // Live snapshot refresh
  private refreshInterval?: number;
  private lastUpdate = signal<number>(Date.now());

  ngOnInit() {
    // Refresh live snapshot every 2 seconds
    this.refreshInterval = window.setInterval(() => {
      if (this.viewMode() === 'live') {
        this.lastUpdate.set(Date.now());
      }
    }, 2000);

    // Listen to route changes for camera selection
    combineLatest([this.cameras$, this.route.params]).subscribe(([cameras, params]) => {
      if (cameras.length === 0) return;

      const cameraId = params['cameraId'];

      // Find camera matching the route
      const camera = cameras.find(c => c.id === cameraId);

      if (camera) {
        const currentCamera = this.selectedCamera();
        const cameraChanged = !currentCamera || currentCamera.id !== camera.id;

        // Update camera if it changed
        if (cameraChanged) {
          this.selectedCamera.set(camera);
          this.loadEvents();
          this.loadPeriodicSnapshots();
          // Reset to live view when switching cameras
          this.viewMode.set('live');
          this.selectedRecordingUrl.set(null);
          this.selectedEventId.set(null);
        }

        this.titleService.setTitle(`YOLO Detection - ${camera.name}`);
      } else if (!this.selectedCamera()) {
        // Fallback to first camera if route param invalid
        this.selectedCamera.set(cameras[0]);
        this.titleService.setTitle(`YOLO Detection - ${cameras[0].name}`);
        this.router.navigate(['/', cameras[0].id], { replaceUrl: true });
        this.loadEvents();
      }
    });
  }


  // Load events for current camera and date
  private loadEvents() {
    const camera = this.selectedCamera();
    const date = this.selectedDate();

    if (!camera) {
      return;
    }

    this.isLoadingEvents.set(true);
    this.eventsService.getEventsByDate(camera.id, date).subscribe({
      next: (events) => {
        this.events.set(events);
        this.isLoadingEvents.set(false);
      },
      error: (error) => {
        console.error('Error loading events:', error);
        this.events.set([]);
        this.isLoadingEvents.set(false);
      }
    });
  }

  // Event handlers
  onCameraChange(camera: Camera) {
    // Navigate to the camera route
    this.router.navigate([camera.id]);
  }

  onDateChange(date: Date) {
    this.selectedDate.set(date);
    this.loadEvents();
    this.loadPeriodicSnapshots();
  }

  onSearch(searchQuery: SearchQuery) {
    this.isSearchMode.set(true);
    this.isSearchLoading.set(true);
    this.searchInfo.set(null);

    this.eventsService.searchEvents(searchQuery.query, {
      classes: searchQuery.classes.length > 0 ? searchQuery.classes.join(',') : undefined
    }).subscribe({
      next: (response) => {
        this.searchResults.set(response.events);
        this.searchInfo.set({ mode: response.search_mode, count: response.count });
        this.isSearchLoading.set(false);
      },
      error: (error) => {
        console.error('Search error:', error);
        this.searchResults.set([]);
        this.isSearchLoading.set(false);
      }
    });
  }

  onSearchClear() {
    this.isSearchMode.set(false);
    this.searchResults.set([]);
    this.searchInfo.set(null);
  }

  onSearchResultSelect(result: SearchResult) {
    if (result.type === 'event' && result.recording_url) {
      const relativePath = toRelativeMediaUrl(result.recording_url, 'events');
      this.selectedEventId.set(result.id);
      this.selectedRecordingUrl.set(relativePath);
      this.viewMode.set('recording');
      this.videoError.set(false);
    } else if (result.snapshot_url) {
      // Show snapshot in viewer
      const relativePath = toRelativeMediaUrl(result.snapshot_url, 'snapshots');
      this.selectedEventId.set(result.id);
      this.selectedRecordingUrl.set(relativePath);
      this.viewMode.set('snapshot');
      this.videoError.set(false);
    }
  }

  onSnapshotSelect(snapshot: PeriodicSnapshot) {
    const relativePath = toRelativeMediaUrl(snapshot.snapshot_url, 'snapshots');
    this.selectedEventId.set(String(snapshot.snapshot_id));
    this.selectedRecordingUrl.set(relativePath);
    this.viewMode.set('snapshot');
    this.videoError.set(false);
  }

  private loadPeriodicSnapshots() {
    const camera = this.selectedCamera();
    const date = this.selectedDate();
    if (!camera) return;

    this.eventsService.getPeriodicSnapshots(camera.id, date).subscribe({
      next: (snapshots) => this.periodicSnapshots.set(snapshots),
      error: () => this.periodicSnapshots.set([])
    });
  }

  onEventSelect(event: DetectionEvent) {
    if (event.recording_url) {
      const relativePath = toRelativeMediaUrl(event.recording_url, 'events');

      // Update state to show recording
      this.selectedEventId.set(event.event_id);
      this.selectedRecordingUrl.set(relativePath);
      this.viewMode.set('recording');
      this.videoError.set(false);
    }
  }

  onBackToLive() {
    // Reset to live view
    this.viewMode.set('live');
    this.selectedRecordingUrl.set(null);
    this.selectedEventId.set(null);
    this.videoError.set(false);
  }

  onVideoError() {
    console.error('Failed to load recording:', this.selectedRecordingUrl());
    this.videoError.set(true);
  }

  // Get live snapshot URL with cache-busting timestamp
  getLiveSnapshotUrl(): string {
    const camera = this.selectedCamera();
    if (!camera) {
      return '';
    }
    return `api/cameras/${camera.id}/snapshot?t=${this.lastUpdate()}`;
  }

  formatTimestamp(ts: string): string {
    const date = new Date(ts);
    return date.toLocaleDateString('en-US', {
      month: 'short', day: 'numeric'
    }) + ' ' + date.toLocaleTimeString('en-US', {
      hour: 'numeric', minute: '2-digit', hour12: true
    });
  }

  getSearchResultSnapshotUrl(result: SearchResult): string {
    if (!result.snapshot_url) return '';
    return toRelativeMediaUrl(result.snapshot_url, 'snapshots');
  }

  ngOnDestroy() {
    if (this.refreshInterval) {
      clearInterval(this.refreshInterval);
    }
  }
}
