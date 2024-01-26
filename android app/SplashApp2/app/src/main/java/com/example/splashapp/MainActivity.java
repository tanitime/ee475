package com.example.splashapp;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;



import java.util.UUID;

@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
public class MainActivity extends Activity {

    private static final String TAG = "BLEExample";
    private static final int REQUEST_ENABLE_BT = 1;
    private static final int PERMISSION_REQUEST_CODE = 2;

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;

    private BluetoothGattCharacteristic bluetoothGattCharacteristic;

    private final String deviceAddress = "08:D1:F9:DE:1E:32"; // Replace with your ESP32's address
    private final UUID serviceUUID = UUID.fromString("20336bc0-c8f9-4de7-b637-a68b7ef33fc9"); // Example service UUID

    private final UUID dataCharacteristicUUID = UUID.fromString("23336bc0-c8f9-4de7-b637-a68b7ef33fc9");

    private TextView connectionStatusTextView;

    private TextView soilMoistureTextView;
    private TextView temperatureTextView;
    private TextView humidityTextView;
    private TextView lightTextView;
    private TextView waterTextView;

    private boolean isConnected;

    private EditText inputTime;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        View connectButton = findViewById(R.id.connectButton);
        View solenoidButton = findViewById(R.id.solenoidButton);
        connectionStatusTextView = findViewById(R.id.connectionStatusTextView);
        inputTime = findViewById(R.id.inputTime);

        // Initialize your TextViews
        soilMoistureTextView = findViewById(R.id.soilMoistureTextView);
        temperatureTextView = findViewById(R.id.temperatureTextView);
        humidityTextView = findViewById(R.id.humidityTextView);
        lightTextView = findViewById(R.id.lightTextView);
        waterTextView = findViewById(R.id.waterTextView);

        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (hasBluetoothPermissions()) {
                    connectToDevice();
                } else {
                    requestPermission();
                }
            }
        });

        solenoidButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isConnected == true) {
                    int timeInSeconds = Integer.parseInt(inputTime.getText().toString());
                    writeToCharacteristic(bluetoothGatt, bluetoothGattCharacteristic, String.valueOf(timeInSeconds));
                    Log.d(TAG, "Clicked Solenoid Button");
                }
            }
        });




        // Check if the device supports BLE
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Log.e(TAG, "BLE is not supported on this device");
            finish();
        }

        // Initialize BluetoothManager and BluetoothAdapter
        final BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        bluetoothAdapter = bluetoothManager.getAdapter();

        // Check if Bluetooth is supported on the device
        if (bluetoothAdapter == null) {
            Log.e(TAG, "Bluetooth is not supported on this device");
            finish();
            return;
        }

        // Check if Bluetooth is enabled, request enabling if not
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        } else {
            connectToDevice();
        }
    }

    private boolean hasBluetoothPermissions() {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) == PackageManager.PERMISSION_GRANTED
                && ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN) == PackageManager.PERMISSION_GRANTED
                && ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED;
    }

    private void requestPermission() {
        ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.BLUETOOTH, Manifest.permission.BLUETOOTH_ADMIN, Manifest.permission.BLUETOOTH_CONNECT},
                PERMISSION_REQUEST_CODE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == PERMISSION_REQUEST_CODE) {
            boolean allPermissionsGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allPermissionsGranted = false;
                    break;
                }
            }

            if (allPermissionsGranted) {
                connectToDevice();
            } else {
                Log.e(TAG, "Permission denied. Exiting...");
                finish();
            }
        }
    }

    private void connectToDevice() {
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
        bluetoothGatt = device.connectGatt(this, false, gattCallback);
    }

    private void writeToCharacteristic(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, String data) {
        if (gatt != null && isConnected) {
            // Check if the Bluetooth device is connected

            if (isConnected) {
                // Check if the characteristic is not null
                if (characteristic != null) {
                    // Check if the characteristic supports write operations
                    int properties = characteristic.getProperties();
                    Log.d(TAG, "Characteristic Properties: " + properties);
                    if ((properties & BluetoothGattCharacteristic.PROPERTY_WRITE) != 0) {
                        // Set the write type of the characteristic
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);

                        // Set the value and write to the characteristic
                        characteristic.setValue(data);
                        boolean success = gatt.writeCharacteristic(characteristic);

                        if (success) {
                            Log.i(TAG, "Write to characteristic successful");
                            Log.i(TAG, data);
                        } else {
                            Log.e(TAG, "Failed to write to characteristic");
                        }
                    } else {
                        Log.e(TAG, "Characteristic does not support write operations");
                    }
                } else {
                    Log.e(TAG, "Characteristic not found");
                }
            } else {
                Log.e(TAG, "Bluetooth device not connected");
            }
        } else {
            Log.e(TAG, "BluetoothGatt not initialized or not connected");
        }
    }


    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (newState == BluetoothGatt.STATE_CONNECTED) {
                        Log.i(TAG, "Connected to GATT server.");
                        connectionStatusTextView.setText("Connected");
                        isConnected = true;
                        gatt.discoverServices();
                    } else if (newState == BluetoothGatt.STATE_DISCONNECTED) {
                        Log.i(TAG, "Disconnected from GATT server.");
                        connectionStatusTextView.setText("Disconnected");
                        isConnected = false;
                    }
                }
            });
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                BluetoothGattService service = gatt.getService(serviceUUID);

                // Assuming you have only one characteristic for writing, adjust accordingly
                bluetoothGattCharacteristic = service.getCharacteristic(dataCharacteristicUUID);

                // Read the characteristic value after discovering services
                gatt.readCharacteristic(bluetoothGattCharacteristic);

                // Enable notifications for the characteristic
                enableNotifications(gatt, bluetoothGattCharacteristic);
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }



        private void enableNotifications(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            // Check if the characteristic has the NOTIFY property
            if ((characteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0) {
                // Get the CCCD descriptor for the characteristic
                BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));

                // Enable notifications locally
                gatt.setCharacteristicNotification(characteristic, true);

                // Enable notifications on the remote device by writing to the CCCD descriptor
                descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                gatt.writeDescriptor(descriptor);
            }
        }


        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                // Handle the characteristic value here
                if (characteristic.getUuid().equals(dataCharacteristicUUID)) {
                    displayCharacteristicValue(characteristic);
                }
//
            } else {
                Log.w(TAG, "onCharacteristicRead received: " + status);
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            // Characteristic notification/indication received
            // Handle the characteristic value here
            if (characteristic.getUuid().equals(dataCharacteristicUUID)) {
                displayCharacteristicValue(characteristic);
            }
//
        }

        private void displayCharacteristicValue(final BluetoothGattCharacteristic characteristic) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    byte[] data = characteristic.getValue();
                    String value = new String(data);
                    String vals[] = {"","","",""};
                    vals = value.split(",", 5);
                    String soilMoistureValue = vals[0];
                    String lightValue = vals[1].substring(0,1);
                    String lightLevel = "";
                    if (lightValue.equals("1") ){
                        lightLevel = "Dark";
                    } else if (lightValue.equals("2")){
                        lightLevel = "Low Light";
                    } else if (lightValue.equals("3")){
                        lightLevel = "Moderate Light";
                    } else if (lightValue.equals("4")){
                        lightLevel = "Bright";
                    } else {
                        lightLevel = "Incorrect Value";
                    }
                    String waterValue = vals[1].substring(1,2);
                    String waterLevel = "";
                    if (waterValue.equals("0")){
                        waterLevel = "Enough Water";
                    } else {
                        waterLevel = "Needs Refilling";
                    }
                    String tempValue = vals[2];
                    String humidityValue = vals[3];

                    // Update the corresponding TextView with the received value
                    if (characteristic.getUuid().equals(dataCharacteristicUUID)) {
                        soilMoistureTextView.setText("Soil Moisture: " + soilMoistureValue + "/4095");
                        lightTextView.setText("Light: " + lightLevel);
                        waterTextView.setText("Water: " + waterLevel);
                        temperatureTextView.setText("Temperature: " + tempValue + "°C");
                        humidityTextView.setText("Humidity: " + humidityValue + "%");

                    }
//
                }
            });
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            // Descriptor write operation completed



            if (status == BluetoothGatt.GATT_SUCCESS) {

                // You can start receiving notifications or indications here
            } else {
                Log.w(TAG, "onDescriptorWrite received: " + status);
            }
        }

    };
}