import numpy as np
import matplotlib.pyplot as plt
import tensorflow as tf

# need to turn off eager execution, because we are using placeholders
# tf.compat.v1.disable_eager_execution()

if tf.version.VERSION == '2.0.0':
    import tensorflow_datasets as tfds
    datasets, info = tfds.load(name='mnist', with_info=True)
    mnist  = datasets['train']
    # tfds.show_examples(info, mnist)
else:
    from tensorflow.examples.tutorials.mnist import input_data
    mnist=input_data.read_data_sets("/MNIST_data/",one_hot=True)

num_inputs=784    #28x28 pixels
num_hid1=392
num_hid2=196
num_hid3=num_hid1
num_output=num_inputs
lr=0.01
actf=tf.nn.relu

X=tf.compat.v1.placeholder(tf.float32,shape=[None,num_inputs])
initializer=tf.compat.v1.variance_scaling_initializer()

w1=tf.Variable(initializer([num_inputs,num_hid1]),dtype=tf.float32)
w2=tf.Variable(initializer([num_hid1,num_hid2]),dtype=tf.float32)
w3=tf.Variable(initializer([num_hid2,num_hid3]),dtype=tf.float32)
w4=tf.Variable(initializer([num_hid3,num_output]),dtype=tf.float32)

b1=tf.Variable(tf.zeros(num_hid1))
b2=tf.Variable(tf.zeros(num_hid2))
b3=tf.Variable(tf.zeros(num_hid3))
b4=tf.Variable(tf.zeros(num_output))

hid_layer1=actf(tf.matmul(X,w1)+b1)
hid_layer2=actf(tf.matmul(hid_layer1,w2)+b2)
hid_layer3=actf(tf.matmul(hid_layer2,w3)+b3)
output_layer=actf(tf.matmul(hid_layer3,w4)+b4)


loss=tf.reduce_mean(input_tensor=tf.square(output_layer-X))

optimizer=tf.compat.v1.train.AdamOptimizer(lr)
train=optimizer.minimize(loss)

init=tf.compat.v1.global_variables_initializer()


num_epoch=5
batch_size=150
num_test_images=10

with tf.compat.v1.Session() as sess:
    # sess.run(init)
    mnist = mnist.batch(batch_size)
    test_batch = mnist.take(1)
    for epoch in range(num_epoch):
                
        num_batches=info.splits['train'].num_examples//batch_size
        for iteration in range(num_batches):
            X_batch = mnist.take(1).as_numpy_iterator()
            sess.run(train,feed_dict={X:X_batch})
            
        train_loss=loss.eval(feed_dict={X:X_batch})
        print("epoch {} loss {}".format(epoch,train_loss))
        
    results=output_layer.eval(feed_dict={X:mnist.test.images[:num_test_images]})
    
    #Comparing original images with reconstructions
    f,a=plt.subplots(2,10,figsize=(20,4))
    for i in range(num_test_images):
        a[0][i].imshow(np.reshape(mnist.test.images[i],(28,28)))
        a[1][i].imshow(np.reshape(results[i],(28,28)))
