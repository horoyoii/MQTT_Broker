# MQTT Broker based on epoll  
  This is an implementation of a server for the MQTT protocol.  
    
  This broker is written in C++, using a **epoll**.  
  Currently this broker serves only QoS 0.
  
   
### Links  
  See the following links for more information on MQTT:
  
  * MQTT v3.1.1 standard : https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/mqtt-v3.1.1.html  
  * MQTT v5.0 standard :  https://docs.oasis-open.org/mqtt/mqtt/v5.0/mqtt-v5.0.html  
  
  Epoll information is available at the following locations:  
  
  * epoll wiki : https://en.wikipedia.org/wiki/Epoll  
  * Linux man page : http://man7.org/linux/man-pages/man7/epoll.7.html  
  
 
Linux's epoll based mqtt broker 


## Introduction  
  
  There are many open sources for a mqtt broker. [eclipse/mosquitto](https://github.com/eclipse/mosquitto) is one of them.   
  But this open source is using a poll/select mode by reason of portability.  
  
  **This topic was discussed in eclipse/mosquitto' issue #449**  
  https://github.com/eclipse/mosquitto/issues/449  
  
  As mentioned above the issue, select system call is hard to handle the [c10k problem](https://en.wikipedia.org/wiki/Select_(Unix))  
  
  So this implemtation is based epoll system call in the Linux as part of the effort to handle the problem.  
  

## Stuff it doesn't do  

  * support only QoS 0  
  * Linux specific  

## Compliation  

   
  


