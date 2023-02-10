
def mqtt_listener():
    

    import paho.mqtt.client as mqtt
    def on_connect(client, userdata, flags, rc):
        print(f"Connected with result code {rc}")
        client.subscribe("room_three")
       
    # the callback function, it will be triggered when receiving messages
    def on_message(client, userdata, msg):
            
        message = msg.payload.decode("utf-8")
       
        if msg.topic == "room_three":
            if message == 'reboot':
                print(message)
                print('rebooting')
                os.system('sudo reboot')
            if message == 'poweroff':
                print(message)
                print('poweroff')
                os.system('sudo poweroff')
       
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # set the will message, when the Raspberry Pi is powered off, or the network is interrupted abnormally, it will send the will message to other clients
    client.will_set('room_three', b'{"status": "Off"}')

    # create connection, the three parameters are broker address, broker port number, and keep-alive time respectively
    client.connect("192.168.1.101", 1883, 60, bind_port=0)
   
    while True:
       
        client.loop()
        
    listener_window.close()
    ###end of listener only code
mqtt_listener()
