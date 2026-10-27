package com.example.autoplant;

import android.Manifest;
import android.annotation.SuppressLint;
import android.bluetooth.*;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import java.util.*;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "BLE";
    private static final int REQUEST_BLUETOOTH_PERMISSIONS = 1;
    private static final long READ_INTERVAL_MS = 1000;
    private static final int GATT_TIMEOUT_MS = 5000;

    private static final UUID ESP32_SERVICE_UUID = UUID.fromString("00000012-3456-7812-3456-781234444444");
    private static final UUID SOLENOID_UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef6");
    private static final UUID TEMP_UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef2");
    private static final UUID MOISTURE_UUID = UUID.fromString("4d6f6973-7475-7265-1234-fb6789444440");
    private static final UUID LIGHT_UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef5");
    private static final UUID PRESSURE_UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef4");
    private static final UUID HUMIDITY_UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef3");

    private TextView temperatureView, moistureView, lightView, pressureView, humidityView;
    private Button scanButton, pumpButton;
    private BluetoothLeScanner bleScanner;
    private BluetoothGatt bluetoothGatt;
    private BluetoothGattCharacteristic solenoidChar;
    private final Queue<BluetoothGattCharacteristic> readQueue = new LinkedList<>();
    private final Handler readHandler = new Handler();
    private Runnable readRunnable;
    private Runnable timeoutRecovery;
    private boolean isGattBusy = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initViews();
        requestPermissionsIfNeeded();
        initBLE();
        setListeners();
    }

    private void initViews() {
        scanButton = findViewById(R.id.scanButton);
        pumpButton = findViewById(R.id.pumpControlButton);
        pumpButton.setEnabled(false);
        temperatureView = findViewById(R.id.temperatureView);
        moistureView = findViewById(R.id.moistureView);
        lightView = findViewById(R.id.lightView);
        pressureView = findViewById(R.id.pressureView);
        humidityView = findViewById(R.id.humidityView);
    }

    private void initBLE() {
        BluetoothManager manager = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = manager != null ? manager.getAdapter() : null;
        bleScanner = adapter != null ? adapter.getBluetoothLeScanner() : null;
    }

    private void setListeners() {
        scanButton.setOnClickListener(v -> startBLEScan());
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            pumpButton.setOnClickListener(v -> {
                if (!isGattBusy) {
                    isGattBusy = true;
                    pumpButton.setEnabled(false);
                    togglePump();
                }
            });
        }
    }

    private boolean hasPermission(String permission) {
        return ActivityCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED;
    }

    @SuppressLint({"InlinedApi", "ObsoleteSdkInt"})
    private boolean hasBluetoothPermissions() {
        return hasPermission(Manifest.permission.BLUETOOTH_CONNECT)
                && hasPermission(Manifest.permission.BLUETOOTH_SCAN)
                && (Build.VERSION.SDK_INT > Build.VERSION_CODES.M
                || hasPermission(Manifest.permission.ACCESS_FINE_LOCATION));
    }

    private void requestPermissionsIfNeeded() {
        if (!hasBluetoothPermissions()) {
            @SuppressLint("ObsoleteSdkInt") String[] permissions = Build.VERSION.SDK_INT <= Build.VERSION_CODES.M
                    ? new String[]{Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.ACCESS_FINE_LOCATION}
                    : new String[]{Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_SCAN};
            ActivityCompat.requestPermissions(this, permissions, REQUEST_BLUETOOTH_PERMISSIONS);
        }
    }

    @SuppressLint("MissingPermission")
    private void startBLEScan() {
        if (bleScanner != null && hasPermission(Manifest.permission.BLUETOOTH_SCAN)) {
            Toast.makeText(this, "Scanning for ESP32...", Toast.LENGTH_SHORT).show();
            bleScanner.startScan(scanCallback);
        }
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @SuppressLint("MissingPermission")
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if (!hasPermission(Manifest.permission.BLUETOOTH_CONNECT)) return;
            BluetoothDevice device = result.getDevice();
            Log.d(TAG, "Discovered: " + device.getName() + " - " + device.getAddress());
            if ("ESP32_Sensor".equals(device.getName())) {
                bleScanner.stopScan(this);
                connectToDevice(device);
            }
        }
    };

    @SuppressLint("MissingPermission")
    private void connectToDevice(BluetoothDevice device) {
        bluetoothGatt = device.connectGatt(this, false, new BluetoothGattCallback() {
            @Override
            public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    bluetoothGatt.discoverServices();
                    runOnUiThread(() -> pumpButton.setEnabled(true));
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    runOnUiThread(() -> pumpButton.setEnabled(false));
                }
            }

            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                BluetoothGattService service = gatt.getService(ESP32_SERVICE_UUID);
                if (service != null) {
                    solenoidChar = service.getCharacteristic(SOLENOID_UUID);
                    readRunnable = () -> queueAllSensorReads(service);
                    timeoutRecovery = () -> {
                        if (isGattBusy) {
                            Log.w(TAG, "GATT timeout, resetting state");
                            isGattBusy = false;
                            readHandler.post(readRunnable);
                        }
                    };
                    readHandler.post(readRunnable);
                }
            }

            @SuppressLint("SetTextI18n")
            @Override
            public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                readHandler.removeCallbacks(timeoutRecovery);
                if (status != BluetoothGatt.GATT_SUCCESS) {
                    Log.e(TAG, "Read failed with status: " + status);
                    isGattBusy = false;
                    return;
                }
                UUID uuid = characteristic.getUuid();
                String value = characteristic.getStringValue(0);
                runOnUiThread(() -> {
                    if (uuid.equals(TEMP_UUID)) temperatureView.setText("Temperature: " + value + "°C");
                    else if (uuid.equals(MOISTURE_UUID)) moistureView.setText("Moisture: " + value + "%");
                    else if (uuid.equals(HUMIDITY_UUID)) humidityView.setText("Humidity(air): " + value + "%");
                    else if (uuid.equals(PRESSURE_UUID)) pressureView.setText("Pressure: " + value + " hPa");
                    else if (uuid.equals(LIGHT_UUID)) lightView.setText("Light: " + value + " lux");
                });
                if (!readQueue.isEmpty()) {
                    bluetoothGatt.readCharacteristic(readQueue.poll());
                    readHandler.postDelayed(timeoutRecovery, GATT_TIMEOUT_MS);
                } else {
                    isGattBusy = false;
                    readHandler.postDelayed(readRunnable, READ_INTERVAL_MS);
                }
            }

            @Override
            public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                readHandler.removeCallbacks(timeoutRecovery);
                isGattBusy = false;
                runOnUiThread(() -> pumpButton.setEnabled(true));
                readHandler.postDelayed(readRunnable, READ_INTERVAL_MS);
            }
        });
    }

    @SuppressLint("MissingPermission")
    private void queueAllSensorReads(BluetoothGattService service) {
        if (isGattBusy) return;
        readQueue.clear();
        Collections.addAll(readQueue,
                safeGetChar(service, TEMP_UUID),
                safeGetChar(service, MOISTURE_UUID),
                safeGetChar(service, HUMIDITY_UUID),
                safeGetChar(service, PRESSURE_UUID),
                safeGetChar(service, LIGHT_UUID)
        );
        readQueue.removeIf(Objects::isNull);
        if (!readQueue.isEmpty() && bluetoothGatt != null) {
            isGattBusy = true;
            bluetoothGatt.readCharacteristic(readQueue.poll());
            readHandler.postDelayed(timeoutRecovery, GATT_TIMEOUT_MS);
        }
    }

    private BluetoothGattCharacteristic safeGetChar(BluetoothGattService service, UUID uuid) {
        BluetoothGattCharacteristic c = service.getCharacteristic(uuid);
        if (c == null) Log.w(TAG, "Missing characteristic: " + uuid);
        return c;
    }

    @SuppressLint("MissingPermission")
    @RequiresApi(api = Build.VERSION_CODES.S)
    private void togglePump() {
        if (solenoidChar != null && bluetoothGatt != null) {
            readHandler.removeCallbacks(readRunnable);
            readQueue.clear();
            String currentState = solenoidChar.getStringValue(0);
            String command = "open".equals(currentState) ? "close" : "open";
            solenoidChar.setValue(command);
            if (!hasPermission(Manifest.permission.BLUETOOTH_CONNECT)) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.BLUETOOTH_CONNECT},
                        REQUEST_BLUETOOTH_PERMISSIONS);
                isGattBusy = false;
                return;
            }
            bluetoothGatt.writeCharacteristic(solenoidChar);
            Toast.makeText(this, "Pump " + command, Toast.LENGTH_SHORT).show();
            readHandler.postDelayed(timeoutRecovery, GATT_TIMEOUT_MS);
        } else {
            Toast.makeText(this, "Unable to control pump. Not connected.", Toast.LENGTH_SHORT).show();
            isGattBusy = false;
        }
    }

    @SuppressLint("MissingPermission")
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (readHandler != null && readRunnable != null) readHandler.removeCallbacks(readRunnable);
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
        }
    }
}