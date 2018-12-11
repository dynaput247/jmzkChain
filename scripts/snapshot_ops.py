#!/usr/bin/env python3

import os
import pathlib
import time

import boto3
import click


def green(text):
    return click.style(text, fg='green')


@click.group('cli')
def cli():
    pass


@cli.command()
@click.option('--file', '-f', type=click.Path(exists=True), required=True)
@click.option('--block-id', '-i', default='')
@click.option('--block-num', '-n', default='')
@click.option('--block-time', '-t', default='')
@click.option('--postgres/--no-postgres', '-p', default=False)
@click.option('--bucket', '-b', default='evt-snapshots')
@click.option('--aws-key', '-k', required=True)
@click.option('--aws-secret', '-s', required=True)
def upload(file, block_id, block_num, block_time, postgres, bucket, aws_key, aws_secret):
    session = boto3.Session(aws_access_key_id=aws_key,
                            aws_secret_access_key=aws_secret)
    s3 = session.resource('s3')

    t = time.monotonic()

    click.echo('Uploading: {} to {} bucket'.format(
        click.style(key, fg='red'), click.style(bucket, fg='green')))
    s3.Object(bucket, key).put(
        Body=open(file, 'rb'),
        Metadata={
            'block-id': block_id,
            'block_num': block_num,
            'block_time': block_time,
            'postgres': postgres
        },
        StorageClass='STANDARD_IA'
    )

    s3.ObjectAcl(bucket, key).put(
        ACL='public-read'
    )

    t2 = time.monotonic()

    click.echo('Uploaded all the symbols, took {} ms'.format(
               click.style(str(round((t2-t) * 1000)), fg='green')))


@cli.command()
@click.option('--name', '-n', required=True)
@click.option('--bucket', '-b', default='evt-snapshots')
@click.option('--file', '-f', required=True)
def fetch(name, bucket, file, aws_key, aws_secret):
    s3 = boto3.resource('s3')

    t = time.monotonic()

    click.echo('Downloading {} from {} bucket'.format(
        click.style(name, fg='red'), click.style(bucket, fg='green')))

    try:
        s3.Bucket(bucket).download_file(name, file)
    except botocore.exceptions.ClientError as e:
        if e.response['Error']['Code'] == "404":
            print("The object does not exist.")
        else:
            raise

    click.echo('Download finished, took {} ms'.format(
               click.style(str(round((t2-t) * 1000)), fg='green')))


@cli.command()
@click.option('--number', '-n', default=10, help='Number of latest snapshots to list')
@click.option('--bucket', '-b', default='evt-snapshots')
def list(number, bucket):
    s3 = boto3.resource('s3')

    objs = s3.Bucket(bucket).objects.limit(count = number)
    
    click.echo('{:<80} {:>8} {:<12}'.format(green('name'), green('number'), green('postgres')))
    for obj in objs:
        name = obj.key
        metas = s3.Object(bucket, name).metadata;
        num = metas['block_num']
        pg = metas['postgres']

        click.echo('{:<80} {:>8} {:<12}'.format(name, num, pg))


if __name__ == '__main__':
    cli()
